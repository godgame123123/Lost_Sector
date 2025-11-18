// Copyright Epic Games, Inc. All Rights Reserved.

#include "LostSectorCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Weapon.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// ALostSectorCharacter

ALostSectorCharacter::ALostSectorCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	
	// 캐릭터 생성 시 기본 Stats 값
	CharacterStats.Hp = 100.0f;
	CharacterStats.Stamina = 100.0f;
	CharacterStats.hungry = 100.0f;
	CharacterStats.weight = 0.0f;

	
}

// Input

void ALostSectorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALostSectorCharacter::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALostSectorCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
	

}

// To add mapping context
inline void ALostSectorCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();


	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		// 마우스 커서 표시
		PlayerController->bShowMouseCursor = true;

		// 마우스 클릭이 월드와 UI에 모두 영향을 미치도록 설정
		PlayerController->SetInputMode(FInputModeGameAndUI());

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	GetWorldTimerManager().SetTimer(
		StaminaTimerHandle,
		this,
		&ALostSectorCharacter::StaminaRegenDrainTick,
		0.1f, // 틱 간격
		true  // 반복
	);

	GetWorldTimerManager().SetTimer(
		HungerTimerHandle,
		this,
		&ALostSectorCharacter::HungerDrainTick,
		0.1f, // 틱 간격
		true
	);
	EquipWeapon();
}
void ALostSectorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 캐릭터 무브먼트 컴포넌트를 가져옵니다.
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();

	if (bIsSprinting)
	{
		// [뛸 때 로직]
		// 움직임 방향으로 회전하도록 Unreal Engine의 기본 기능(bOrientRotationToMovement)을 활성화합니다.
		if (MovementComp && !MovementComp->bOrientRotationToMovement)
		{
			MovementComp->bOrientRotationToMovement = true;
		}

		// 달리는 중에는 마우스 회전 로직을 건너뜁니다.
		return;
	}
	else // 걷거나 멈춰있을 때 (bIsSprinting == false)
	{
		// [걷거나 멈춰있을 때 로직]
		// 마우스 커서 방향으로 수동 회전하기 위해 Unreal Engine의 자동 회전 기능을 비활성화합니다.
		if (MovementComp && MovementComp->bOrientRotationToMovement)
		{
			MovementComp->bOrientRotationToMovement = false;
		}

		// --- 마우스 커서 방향으로 회전시키는 기존 수동 로직 시작 ---

		APlayerController* PC = Cast<APlayerController>(GetController());
		if (!PC)
		{
			return;
		}

		FVector WorldLocation, WorldDirection;
		PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

		FHitResult HitResult;
		FVector StartTrace = WorldLocation;
		FVector EndTrace = WorldLocation + WorldDirection * 50000.0f;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartTrace,
			EndTrace,
			ECollisionChannel::ECC_WorldStatic,
			Params
		);

		FVector TargetLocation = bHit ? HitResult.Location : EndTrace;

		// 현재 위치에서 타겟 위치를 바라보는 방향을 계산합니다. (Z축 무시)
		FVector CurrentLocation = GetActorLocation();
		FVector Direction = TargetLocation - CurrentLocation;
		Direction.Z = 0.0f;
		Direction.Normalize();

		FRotator TargetRotation = Direction.Rotation();

		FRotator CurrentRotation = GetActorRotation();
		float RotationSpeed = 10.0f;

		// 부드럽게 보간하여 회전을 적용합니다.
		FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			TargetRotation,
			DeltaTime,
			RotationSpeed
		);

		// 캐릭터의 회전을 Yaw 값으로 업데이트합니다.
		SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));

		// --- 마우스 커서 방향으로 회전시키는 기존 수동 로직 끝 ---
	}
}
bool ALostSectorCharacter::ConsumeStamina(float StaminaCost)
{
	// 1. Stamina가 비용보다 크거나 같은지 확인
	if (CharacterStats.Stamina >= StaminaCost)
	{
		// 2. Stamina 감소
		CharacterStats.Stamina -= StaminaCost;
		// (선택) 디버깅 로그 출력
		UE_LOG(LogTemp, Warning, TEXT("Stamina Consumed: %f	. Current Stamina: %f"), StaminaCost, CharacterStats.Stamina);
		// 3. Stamina 소모 성공
		return true;
	}
	// Stamina가 부족하여 소모 실패
	return false;
}
void ALostSectorCharacter::SetIsSprinting(bool bNewState)
{
	// 달리기 상태가 해제될 때 (True -> False)
	if (bIsSprinting == true && bNewState == false)
	{
		// 현재 월드 시간을 기록합니다.
		LastSprintEndTime = GetWorld()->GetTimeSeconds();
	}
	bIsSprinting = bNewState;
}
void ALostSectorCharacter::StaminaRegenDrainTick()
{
	// MaxStamina 값을 FCharacterData에 추가하지 않았다면, 임시 Max 값 사용 (예시)
	const float MaxStamina = 100.0f;
	float StaminaChange = 0.0f;
	// 현재 월드 시간 가져오기
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	// Stamina 재생 지연 시간 (초)
	const float LastDrainTime = FMath::Max(LastSprintEndTime, LastStaminaZeroTime);
	// 재생 지연 시간 계산
	const float RegenAllowedTime = LastDrainTime + StaminaRegenDelayDuration;
	// 달리기 상태 체크 및 Stamina 변경량 결정
	if (bIsSprinting)
	{
		// 달릴 때: Stamina 소모 (예: 틱당 -1)
		StaminaChange = -1.0f;
	}
	else // 평소 상태 (재생 시도)
	{
		// Stamina가 Max보다 작을 때만 재생 시도
		if (CharacterStats.Stamina < MaxStamina)
		{
			// 지연 시간이 지났는지 확인
			if (CurrentTime < RegenAllowedTime)
			{
				StaminaChange = 0.0f; // 지연 시간 이내: 재생 막음
			}
			else
			{
				StaminaChange = 1.0f; // 지연 시간 경과: Stamina 재생
			}
		}
	}
	// FCharacterData::Stamina 값 업데이트
	CharacterStats.Stamina += StaminaChange;

	// 최소/최대 값으로 Clamp (0 이하, Max 이상으로 넘어가지 않게 방지)
	CharacterStats.Stamina = FMath::Clamp(CharacterStats.Stamina, 0.0f, MaxStamina);

	// Stamina가 0에 도달했을 때의 로직
	if (CharacterStats.Stamina <= 0.0f)
	{
		// 강제 워킹 로직 (Stamina가 0이 되면 달리기 상태 해제)
		if (bIsSprinting)
		{
			if (GetCharacterMovement())
			{
				GetCharacterMovement()->MaxWalkSpeed = 500.0f;
			}
			bIsSprinting = false;
		}
		// Stamina가 0인 상태를 유지할 때 LastStaminaZeroTime을 현재 시간으로 갱신
		// (LastStaminaZeroTime을 계속 갱신하여 0인 상태에서는 재생이 영원히 차단되게 합니다.)
		if (LastStaminaZeroTime == 0.0f)
		{
			LastStaminaZeroTime = CurrentTime;
		}
	}
	// Stamina가 1 이상으로 올라오면 (재생 가능 상태가 되면) LastStaminaZeroTime 초기화
	// (이렇게 해야 지연 시간이 끝난 후 재생이 시작됨과 동시에 LastStaminaZeroTime이 갱신되지 않습니다.)
	else
	{
		LastStaminaZeroTime = 0.0f; // Stamina가 0이 아닐 때는 초기화
	}
}

void ALostSectorCharacter::HungerDrainTick()
{
	// 1. Hunger 소모 로직
	if (CharacterStats.hungry > 0.0f) // 0.0f와 비교
	{
		// 틱당 소모량 (0.05f)만큼 직접 감소
		CharacterStats.hungry -= HungerDrainPerTick;

		// 0.0f 미만으로 내려가지 않도록 Clamp
		CharacterStats.hungry = FMath::Max(0.0f, CharacterStats.hungry);
	}

	// 2. Hunger가 0일 때 HP 감소 로직
	if (CharacterStats.hungry <= 0.0f) // 0.0f와 비교
	{
		// HP가 0.0f보다 클 때만 HP 감소
		if (CharacterStats.Hp > 0.0f) // 0.0f와 비교
		{
			// HealthDrainPerTick은 int32이지만 float 연산에 문제 없음
			CharacterStats.Hp -= HealthDrainPerTick;

			// HP가 0.0f 미만으로 내려가지 않도록 Clamp
			CharacterStats.Hp = FMath::Max(0.0f, CharacterStats.Hp);

			// (선택) 로그 출력 시 포맷 지정자 %f로 변경
			UE_LOG(LogTemp, Warning, TEXT("Hunger 0! Health reduced. Current HP: %f"), CharacterStats.Hp);
		}
	}
}

void ALostSectorCharacter::EquipWeapon()
{
	if (!DefaultWeaponClass)
	{
		return;
	}

	// 1. 무기 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this; // 캐릭터를 Owner로 설정
	SpawnParams.Instigator = GetInstigator();

	CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, SpawnParams);

	if (CurrentWeapon)
	{
		// 2. 캐릭터의 스켈레탈 메쉬에 부착
		if (USkeletalMeshComponent* CharacterMesh = GetMesh())
		{
			const FName WeaponSocketName = FName("WeaponSocket"); // <- 스켈레탈 메쉬의 소켓 이름으로 변경하세요!

			CurrentWeapon->AttachToComponent(
				CharacterMesh,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				WeaponSocketName
			);

			// 3. 무기의 Instigator를 이 캐릭터로 설정 (데미지 ApplyDamage에서 사용됨)
			CurrentWeapon->SetInstigator(this);
		}
	}
}

void ALostSectorCharacter::StartFire()
{

	if (bIsSprinting)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: Cannot fire while sprinting."));
		return;
	}

	if (!CurrentWeapon)
	{
		return;
	}

	// 1. 플레이어 컨트롤러를 가져옵니다.
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	// 2. 마우스 커서의 스크린 위치를 월드 좌표계의 광선(Ray)으로 변환합니다.
	FVector WorldLocation, WorldDirection;
	PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	// 3. 마우스 광선으로 지면을 향해 트레이스하여 타겟 지면 좌표를 찾습니다.
	FHitResult HitResult;
	FVector StartTrace = WorldLocation;
	// 트레이스 길이는 충분히 길게 설정합니다.
	FVector EndTrace = WorldLocation + WorldDirection * 50000.0f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// ECollisionChannel::ECC_WorldStatic 채널로 트레이스하여 지면만 탐지합니다.
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartTrace,
		EndTrace,
		ECollisionChannel::ECC_WorldStatic,
		Params
	);

	FVector TargetLocation = bHit ? HitResult.Location : EndTrace; // 지면에 닿았으면 닿은 위치, 아니면 트레이스 끝점

	// 4. 총구 위치를 가져옵니다. (AWeapon에 GetMuzzleLocation() 함수가 있어야 합니다. 아래 참고)
	USceneComponent* MuzzleComp = CurrentWeapon->GetMuzzleLocation();
	if (!MuzzleComp)
	{
		return;
	}
	FVector MuzzleLocation = MuzzleComp->GetComponentLocation();

	// 5. 총구 위치에서 타겟 지점을 바라보는 방향을 최종 발사 방향으로 계산합니다.
	FVector FireDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();

	// 6. 계산된 방향으로 Fire 함수 호출
	CurrentWeapon->Fire(FireDirection);
}

void ALostSectorCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ALostSectorCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}