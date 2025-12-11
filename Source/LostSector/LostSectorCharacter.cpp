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
#include "MyPlayerState.h"
#include "InventoryComponent.h"
#include "InventorySaveManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"
#include "DeathDropBox.h"
#include "Blueprint/UserWidget.h"
#include "UMiniMapWidget.h"
#include "AMiniMapCapture.h"
#include "Engine/TextureRenderTarget2D.h"


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

	// 스프린트 속도 설정
	SprintSpeed = 800.0f;
	WalkSpeed = 500.0f;

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
	
	// ĳ���� ���� �� �⺻ Stats ��
	CharacterStats.Hp = 100.0f;
	CharacterStats.Stamina = 100.0f;
	CharacterStats.hungry = 100.0f;
	CharacterStats.weight = 0.0f;

	// [ReloadTextWidgetComponent 생성 및 설정]
	ReloadTextWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ReloadTextWidget"));
	ReloadTextWidgetComponent->SetupAttachment(RootComponent); // 루트 컴포넌트에 부착

	// 캐릭터 머리 위 (예: Z축 110.0f)에 위치 조정
	ReloadTextWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));

	// 기본적으로 숨김
	ReloadTextWidgetComponent->SetVisibility(false);

	// 위젯이 항상 카메라를 향하도록 설정 (선택 사항)
	ReloadTextWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	ReloadTextWidgetComponent->SetTwoSided(true);
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
		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ALostSectorCharacter::Reload);
		}
		// Fire - 블루프린트에서 처리하므로 C++ 바인딩 제거
		// 블루프린트에서 Enhanced Input Action 노드를 사용하여 처리
		// if (FireAction)
		// {
		// 	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ALostSectorCharacter::StartFire);
		// 	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ALostSectorCharacter::StopFire);
		// }
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
	
}

// To add mapping context
void ALostSectorCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();


	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->bShowMouseCursor = true;

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
		0.1f, 
		true  
	);

	GetWorldTimerManager().SetTimer(
		HungerTimerHandle,
		this,
		&ALostSectorCharacter::HungerDrainTick,
		0.1f,
		true
	);
	EquipWeapon();
	
	// 미니맵 초기화 (플레이어만)
	if (IsPlayerControlled())
	{
		InitializeMiniMap();
	}
}
float ALostSectorCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 1. 피해를 준 주체(Instigator)가 AI Controller인지 확인
	bool bInstigatorIsAI = EventInstigator && EventInstigator->IsA<AAIController>();

	// 2. 피해를 받은 대상(현재 캐릭터, this)이 플레이어의 제어를 받지 않고 있는지 확인 (즉, AI/NPC)
	bool bTargetIsAIControlled = !IsPlayerControlled();

	// 만약 AI가 AI에게 공격했다면 데미지 처리를 무시하고 0을 리턴
	if (bInstigatorIsAI && bTargetIsAIControlled)
	{
		return 0.0f;
	}
	// 1. ACharacter의 기본 TakeDamage 함수를 호출하여 기본 처리를 수행하고 실제 적용될 데미지량을 얻습니다.
	const float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 2. 이미 죽었거나 적용된 데미지가 없으면 리턴
	if (bIsDead || DamageApplied <= 0.0f)
	{
		return 0.0f;
	}

	// 3. 체력 감소
	CharacterStats.Hp -= DamageApplied;

	// 4. 체력을 0 이상으로 클램프
	CharacterStats.Hp = FMath::Max(0.0f, CharacterStats.Hp);

	UE_LOG(LogTemp, Warning, TEXT("Character %s took %f damage. Current HP: %f"), *GetName(), DamageApplied, CharacterStats.Hp);

	// 5. 사망 체크
	if (CharacterStats.Hp <= 0.0f)
	{
		Die(); // 이미 정의된 Die() 함수 호출
	}

	return DamageApplied;
}
void ALostSectorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UCharacterMovementComponent* MovementComp = GetCharacterMovement();

	if (IsPlayerControlled() && !bIsDead) // 플레이어 제어 중, 살아있을 때만 실행
	{
		HandleOcclusionFade();
		
		// 미니맵 업데이트
		UpdateMiniMap();

		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC && GetMesh())
		{
			FHitResult HitResult;
			FVector MouseWorldLocation, MouseWorldDirection;

			// 1. 마우스 커서를 월드 광선(Ray)으로 변환
			if (PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
			{
				// 2. 광선이 월드 표면에 닿는 지점을 찾기 (Line Trace)
				FVector TraceStart = MouseWorldLocation;
				FVector TraceEnd = MouseWorldLocation + (MouseWorldDirection * 20000.0f); // 충분히 긴 길이 설정

				FCollisionQueryParams Params;
				Params.bReturnPhysicalMaterial = false;
				Params.AddIgnoredActor(this); // 캐릭터는 무시

				// ECC_Visibility 채널을 사용하거나 (Occlusion처럼) ECC_WorldStatic을 사용합니다.
				if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, Params))
				{
					FVector TargetLocation = HitResult.Location;

					// 3. 캐릭터의 머리 위치 (스켈레탈 메시의 "head" 소켓 사용)
					// "head" 소켓이 없다면 "neck_01" 등 가장 가까운 본을 사용해야 합니다.
					FVector HeadLocation = GetMesh()->GetSocketLocation(FName("head"));

					// 4. 타겟 벡터 계산 (머리 -> 마우스 타겟)
					FVector TargetVector = TargetLocation - HeadLocation;

					// 5. 수평 거리와 수직 거리를 사용하여 Pitch 각도 계산

					// 수평 거리 (XY 평면의 크기)
					float DistanceXY = FVector(TargetVector.X, TargetVector.Y, 0.0f).Size();

					// Pitch 각도 계산: ArcTan2(수직 차이, 수평 거리)
					// Atan2(Y, X) 함수를 사용하여 라디안 값을 얻습니다.
					float AngleRad = FMath::Atan2(TargetVector.Z, DistanceXY);

					// 라디안을 도(Degree)로 변환
					float AngleDeg = FMath::RadiansToDegrees(AngleRad);

					// 6. 각도 제한 및 HeadPitch 변수에 저장
					// Aim Offset이 받는 피치 각도를 보통 -45도(숙임) ~ +45도(들음) 사이로 제한합니다.
					HeadPitch = FMath::Clamp(AngleDeg, -80.0f, 80.0f);
				}
			}
		}
	}
	// 스프린트 중 회전 설정
	if (bIsSprinting)
	{
		// 스프린트 중에는 이동 방향으로 자동 회전
		if (MovementComp && !MovementComp->bOrientRotationToMovement)
		{
			MovementComp->bOrientRotationToMovement = true;
		}
		// 스프린트 중에는 마우스 방향 회전 로직을 실행하지 않음 (이동 방향으로 자동 회전)
		return;
	}
	else 
	{
		// 걷기 중에는 마우스 방향으로 회전
		if (MovementComp && MovementComp->bOrientRotationToMovement)
		{
			MovementComp->bOrientRotationToMovement = false;
		}

		APlayerController* PC = Cast<APlayerController>(GetController());
		if (!PC)
		{
			return;
		}

		FInputModeGameAndUI InputMode;
		// 마우스 커서를 가두지 않도록 설정 (선택적)
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);

		PC->bShowMouseCursor = true;

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

		
		FVector CurrentLocation = GetActorLocation();
		FVector Direction = TargetLocation - CurrentLocation;
		Direction.Z = 0.0f;
		Direction.Normalize();

		FRotator TargetRotation = Direction.Rotation();

		FRotator CurrentRotation = GetActorRotation();
		float RotationSpeed = 10.0f;

		FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			TargetRotation,
			DeltaTime,
			RotationSpeed
		);

		FRotator FinalRotation = FRotator(0.0f, NewRotation.Yaw, 0.0f);
		
		// 로컬 플레이어는 즉시 회전 적용
		if (IsLocallyControlled())
		{
			SetActorRotation(FinalRotation);
			
			// 서버로 회전 정보 전송 (변경이 있을 때만)
			if (GetLocalRole() < ROLE_Authority)
			{
				Server_UpdateRotation(FinalRotation);
			}
		}
		else
		{
			// 클라이언트에서는 서버에서 리플리케이트된 회전 정보 사용 (로컬 플레이어 제외)
			if (ReplicatedRotation != FRotator::ZeroRotator)
			{
				SetActorRotation(FRotator(0.0f, ReplicatedRotation.Yaw, 0.0f));
			}
		}
		
		// 서버에서 회전 정보 리플리케이트
		if (HasAuthority()) 
		{
			ReplicatedRotation = FinalRotation;
		}
	}
                                                                                                    }
void ALostSectorCharacter::SetActorOpacity(UPrimitiveComponent* MeshComp, float TargetOpacity)
{
	if (!MeshComp || !IsValid(MeshComp)) return; // MeshComp 유효성 확인 추가

	for (int32 i = 0; i < MeshComp->GetNumMaterials(); ++i)
	{
		UMaterialInterface* Material = MeshComp->GetMaterial(i);

		// Material이 유효한지 확인
		if (!Material) continue;

		UMaterialInstanceDynamic* DynamicMat = Cast<UMaterialInstanceDynamic>(Material);

		if (!DynamicMat)
		{
			// CreateAndSetMaterialInstanceDynamic가 실패할 수도 있으므로 확인
			DynamicMat = MeshComp->CreateAndSetMaterialInstanceDynamic(i);
		}

		if (DynamicMat && IsValid(DynamicMat)) // DynamicMat 유효성 확인
		{
			DynamicMat->SetScalarParameterValue(FName("Opacity"), TargetOpacity);
		}
	}
}
void ALostSectorCharacter::HandleOcclusionFade()
{
	if (!GetFollowCamera() || !GetCapsuleComponent()) return;

	// 1. Line Trace 위치 설정 (카메라 위치에서 캐릭터 중앙까지)
	FVector PlayerLocation = GetActorLocation() + FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.5f);
	FVector CameraLocation = GetFollowCamera()->GetComponentLocation();

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bReturnPhysicalMaterial = false;

	// Line Trace 실행 (카메라에서 플레이어까지)
	GetWorld()->LineTraceMultiByChannel(
		HitResults,
		CameraLocation,
		PlayerLocation,
		ECollisionChannel::ECC_Visibility,
		Params
	);

	//UE_LOG(LogTemp, Warning, TEXT("Total Hits: %d"), HitResults.Num());

	// 디버그 로그 제거 (성능 및 로그 스팸 방지)
	// for (const FHitResult& Hit : HitResults)
	// {
	// 	if (Hit.GetActor())
	// 	{
	// 		// ➡️ 충돌한 모든 액터의 이름 출력
	// 		UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s - Component: %s"),
	// 			*Hit.GetActor()->GetName(),
	// 			*Hit.GetComponent()->GetName()
	// 		);
	// 	}
	// }
	// 현재 프레임에서 Line Trace에 걸린 모든 액터를 저장할 집합
	TSet<AActor*> CurrentOccludingActorsSet; // TSet을 사용하여 중복을 제거합니다.

	// --- 단계 A: 현재 가리는 모든 액터를 투명화 ---
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !HitActor->IsA<ACharacter>() && HitActor->GetRootComponent() && HitActor->GetRootComponent()->Mobility != EComponentMobility::Movable)
		{
			TArray<UPrimitiveComponent*> PrimitiveComponents;
			HitActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

			if (PrimitiveComponents.Num() > 0)
			{
				// ➡️ 여기에서 한 번만 추가
				CurrentOccludingActorsSet.Add(HitActor);

				// 투명화 적용
				for (UPrimitiveComponent* Component : PrimitiveComponents)
				{
					SetActorOpacity(Component, 0.1f);
				}
			}
		}
	}

	// --- 단계 B: 더 이상 가려지지 않는 액터 복구 ---
	// ActorsToRestoreOpacity (이전에 투명했던 액터)를 순회하며 복구 대상을 찾습니다.
	TArray<AActor*> ActorsToUnfade;

	// TArray인 ActorsToRestoreOpacity를 순회
	for (AActor* ActorToRestore : ActorsToRestoreOpacity)
	{
		if (!ActorToRestore || !IsValid(ActorToRestore)) continue;
		// 1. 현재 Line Trace에 잡힌 액터 목록에 없다면 (더 이상 가리지 않는다면)
		if (!CurrentOccludingActorsSet.Contains(ActorToRestore))
		{
			ActorsToUnfade.Add(ActorToRestore);

			// 2. 불투명 복구 로직 실행
			TArray<UPrimitiveComponent*> PrimitiveComponents;
			ActorToRestore->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

			for (UPrimitiveComponent* Component : PrimitiveComponents)
			{
				SetActorOpacity(Component, 1.0f); // 불투명 복구
			}
		}
	}
	// 3. 복구 완료된 액터들을 ActorsToRestoreOpacity 목록에서 제거
	for (AActor* ActorToUnfade : ActorsToUnfade)
	{
		ActorsToRestoreOpacity.Remove(ActorToUnfade);
	}
	for (AActor* CurrentActor : CurrentOccludingActorsSet)
	{
		if (!ActorsToRestoreOpacity.Contains(CurrentActor))
		{
			ActorsToRestoreOpacity.Add(CurrentActor);
		}
	}
}
void ALostSectorCharacter::SetReloadingTextVisible(bool bShow)
{
	if (ReloadTextWidgetComponent)
	{
		ReloadTextWidgetComponent->SetVisibility(bShow);
	}
}

void ALostSectorCharacter::Multicast_SetReloadingTextVisible_Implementation(bool bShow)
{
	// 모든 클라이언트에서 재장전 텍스트 표시/숨김
	SetReloadingTextVisible(bShow);
}
bool ALostSectorCharacter::ConsumeStamina(float StaminaCost)
{
	if (CharacterStats.Stamina >= StaminaCost)
	{
		CharacterStats.Stamina -= StaminaCost;

		UE_LOG(LogTemp, Warning, TEXT("Stamina Consumed: %f	. Current Stamina: %f"), StaminaCost, CharacterStats.Stamina);

		return true;
	}
	return false;
}
void ALostSectorCharacter::SetIsSprinting(bool bNewState)
{
	// 클라이언트에서 서버로 RPC 호출
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_SetIsSprinting(bNewState);
		return;
	}

	// 서버에서 실제 로직 실행
	Server_SetIsSprinting(bNewState);
}

bool ALostSectorCharacter::Server_SetIsSprinting_Validate(bool bNewState)
{
	return true;
}

void ALostSectorCharacter::Server_SetIsSprinting_Implementation(bool bNewState)
{
	if (bIsSprinting == true && bNewState == false)
	{
		LastSprintEndTime = GetWorld()->GetTimeSeconds();
		// 스프린트 종료 시 걷기 속도로 복원
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}
	}
	else if (bIsSprinting == false && bNewState == true)
	{
		// 스프린트 시작 시 속도 증가
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		}
	}
	bIsSprinting = bNewState;
}

void ALostSectorCharacter::OnRep_IsSprinting()
{
	// 클라이언트에서 복제된 값에 따라 속도 업데이트
	if (GetCharacterMovement())
	{
		if (bIsSprinting)
		{
			GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}
	}
}
void ALostSectorCharacter::StaminaRegenDrainTick()
{
	if (!IsPlayerControlled())
	{
		return;
	}
	const float MaxStamina = 100.0f;
	float StaminaChange = 0.0f;

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	const float LastDrainTime = FMath::Max(LastSprintEndTime, LastStaminaZeroTime);

	const float RegenAllowedTime = LastDrainTime + StaminaRegenDelayDuration;

	if (bIsSprinting)
	{
		StaminaChange = -1.0f;
	}
	else 
	{

		if (CharacterStats.Stamina < MaxStamina)
		{

			if (CurrentTime < RegenAllowedTime)
			{
				StaminaChange = 0.0f;
			}
			else
			{
				StaminaChange = 1.0f;
			}
		}
	}
	CharacterStats.Stamina += StaminaChange;

	CharacterStats.Stamina = FMath::Clamp(CharacterStats.Stamina, 0.0f, MaxStamina);

	if (CharacterStats.Stamina <= 0.0f)
	{
		if (bIsSprinting)
		{
			// SetIsSprinting을 호출하여 서버로 RPC 전송 및 복제
			SetIsSprinting(false);
		}
		if (LastStaminaZeroTime == 0.0f)
		{
			LastStaminaZeroTime = CurrentTime;
		}
	}
	else
	{
		LastStaminaZeroTime = 0.0f;
	}
}

void ALostSectorCharacter::HungerDrainTick()
{
	if (!IsPlayerControlled())
	{
		return;
	}
	if (CharacterStats.hungry > 0.0f) 
	{
		CharacterStats.hungry -= HungerDrainPerTick;

		CharacterStats.hungry = FMath::Max(0.0f, CharacterStats.hungry);
	}

	if (CharacterStats.hungry <= 0.0f)
	{
		if (CharacterStats.Hp > 0.0f)
		{
			CharacterStats.Hp -= HealthDrainPerTick;

			CharacterStats.Hp = FMath::Max(0.0f, CharacterStats.Hp);

			UE_LOG(LogTemp, Warning, TEXT("Hunger 0! Health reduced. Current HP: %f"), CharacterStats.Hp);
		}
	}
	if (CharacterStats.Hp <= 0.0f && !bIsDead)
	{
		Die();
	}
}

void ALostSectorCharacter::EquipWeapon()
{
	if (!DefaultWeaponClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this; 
	SpawnParams.Instigator = GetInstigator();

	CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, SpawnParams);

	if (CurrentWeapon)
	{
		// 1. 현재 탄창을 항상 최대치로 채웁니다. (AI/플레이어 공통)
		CurrentWeapon->CurrentAmmo = CurrentWeapon->MaxAmmo;

		// 2. 인벤토리 컴포넌트를 가져옵니다.
		UInventoryComponent* InventoryComp = FindComponentByClass<UInventoryComponent>();

		// 3. 인벤토리 컴포넌트가 있고, 무기가 요구하는 탄약 데이터가 설정되어 있다면
		if (InventoryComp && CurrentWeapon->RequiredAmmoItemData)
		{
			// 플레이어와 AI 모두에게 초기 여분 탄약을 지급합니다.
			const int32 InitialSpareAmmo = 30;

			// FItemStack을 생성하여 TryAddStack에 전달합니다.
			FItemStack AmmoStack;
			AmmoStack.Item = CurrentWeapon->RequiredAmmoItemData;
			// ItemData에서 ItemId를 안전하게 가져옵니다.
			if (CurrentWeapon->RequiredAmmoItemData)
			{
				AmmoStack.ItemId = CurrentWeapon->RequiredAmmoItemData->ItemId;
			}
			AmmoStack.Count = InitialSpareAmmo;

			int32 AddedCount = 0;

			// 인벤토리에 총알 추가 시도 (TryAddStack 사용)
			if (InventoryComp->TryAddStack(AmmoStack, AddedCount))
			{
				UE_LOG(LogTemp, Log, TEXT("%s Equipped Weapon and added %d spare ammo to inventory."),
					*GetName(), AddedCount);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s Equipped Weapon but FAILED to add spare ammo. Check Inventory capacity or ItemData."), *GetName());
			}
			OnAmmoUpdated();
		}

		if (USkeletalMeshComponent* CharacterMesh = GetMesh())
		{
			const FName WeaponSocketName = FName("WeaponSocket");

			CurrentWeapon->AttachToComponent(
				CharacterMesh,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				WeaponSocketName
			);

			CurrentWeapon->SetInstigator(this);
		}

		// 모든 클라이언트에서 무기 장착 시각적 효과 재생
		if (HasAuthority())
		{
			Multicast_EquipWeapon();
		}
	}
}

void ALostSectorCharacter::StartFire()
{
	// 구르기 중이면 발사 불가 (최우선 체크 - 클라이언트/서버 모두)
	// 네트워크 복제 지연을 고려하여 로컬에서도 즉시 차단
	if (Rolling)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("StartFire: Blocked - Character is rolling (Role: %d)"), (int32)GetLocalRole());
		// 발사 타이머가 돌고 있으면 즉시 중지
		if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(FireTimerHandle);
			UE_LOG(LogTemp, Log, TEXT("StartFire: Cleared fire timer due to rolling."));
		}
		// 클라이언트에서도 서버로 RPC를 보내지 않도록 완전히 차단
		return;
	}

	// 클라이언트에서 서버로 RPC 호출 (타겟 위치 계산 후 전달)
	if (GetLocalRole() < ROLE_Authority)
	{
		// 다시 한 번 구르기 체크 (네트워크 복제 지연 대비)
		if (Rolling)
		{
			UE_LOG(LogTemp, Warning, TEXT("StartFire: Blocked on client - Character started rolling before RPC."));
			return;
		}

		FVector TargetLocation = FVector::ZeroVector;
		
		// 클라이언트에서 타겟 위치 계산
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FVector WorldLocation, WorldDirection;
			if (PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
			{
				// 라인 트레이스 실행
				FHitResult HitResult;
				FVector StartTrace = WorldLocation;
				FVector EndTrace = WorldLocation + WorldDirection * (CurrentWeapon ? CurrentWeapon->MaxRange : 5000.0f);

				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				if (GetWorld()->LineTraceSingleByChannel(
					HitResult,
					StartTrace,
					EndTrace,
					ECollisionChannel::ECC_Visibility,
					Params))
				{
					TargetLocation = HitResult.Location;
				}
				else
				{
					TargetLocation = EndTrace;
				}
			}
		}
		
		// 마지막 체크: RPC 전송 직전에 다시 한 번 구르기 상태 확인
		if (!Rolling)
		{
			Server_StartFire(TargetLocation);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("StartFire: Blocked - Character started rolling just before RPC send."));
		}
		return;
	}

	// 서버 플레이어의 경우 타겟 위치를 직접 계산
	// 서버 플레이어도 구르기 체크 (이중 안전장치)
	if (Rolling)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartFire: Blocked on server player - Character is rolling."));
		if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(FireTimerHandle);
		}
		return;
	}

	FVector TargetLocation = FVector::ZeroVector;
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FVector WorldLocation, WorldDirection;
		if (PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			FHitResult HitResult;
			FVector StartTrace = WorldLocation;
			FVector EndTrace = WorldLocation + WorldDirection * (CurrentWeapon ? CurrentWeapon->MaxRange : 5000.0f);

			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			if (GetWorld()->LineTraceSingleByChannel(
				HitResult,
				StartTrace,
				EndTrace,
				ECollisionChannel::ECC_Visibility,
				Params))
			{
				TargetLocation = HitResult.Location;
			}
			else
			{
				TargetLocation = EndTrace;
			}
		}
	}
	
	// 마지막 체크: 서버 플레이어도 RPC 전송 직전에 다시 한 번 확인
	if (!Rolling)
	{
		Server_StartFire(TargetLocation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("StartFire: Blocked on server player - Character started rolling just before RPC."));
	}
}

bool ALostSectorCharacter::Server_StartFire_Validate(FVector ClientTargetLocation)
{
	// 구르기 중이면 검증 실패
	if (Rolling)
	{
		return false;
	}
	return true;
}

void ALostSectorCharacter::Server_StartFire_Implementation(FVector ClientTargetLocation)
{
	// 구르기 중이면 완전히 차단 (최우선 체크)
	if (Rolling)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_StartFire: Blocked - Character is rolling. Clearing any active fire timer."));
		// 발사 타이머가 돌고 있으면 즉시 중지
		if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(FireTimerHandle);
		}
		// 발사 정지 상태 확보
		Server_StopFire();
		return;
	}

	// 스프린트 중이거나 무기가 없으면 발사 불가
	if (bIsSprinting || !CurrentWeapon)
	{
		return;
	}

	AController* CurrentController = GetController();
	FVector FireDirection = GetActorForwardVector();

	// 컨트롤러가 없으면 발사하지 않습니다. (예: 아직 빙의되지 않은 상태)
	if (!CurrentController)
	{
		return;
	}

	USceneComponent* MuzzleComp = CurrentWeapon->GetMuzzleLocation();
	if (!MuzzleComp) return;
	FVector MuzzleLocation = MuzzleComp->GetComponentLocation();

	FVector TargetLocation = FVector::ZeroVector;

	// 1. 플레이어 컨트롤러 조준 로직
	if (CurrentController->IsPlayerController())
	{
	// 클라이언트에서 전달받은 타겟 위치 사용 (클라이언트 플레이어의 경우)
		
		// 또는 서버 플레이어의 경우 직접 계산
		if (ClientTargetLocation != FVector::ZeroVector)


		{
			// 클라이언트에서 전달받은 타겟 위치 사용
			TargetLocation = ClientTargetLocation;
		}
		else
		{
			// 서버 플레이어의 경우 직접 계산
			APlayerController* PC = CastChecked<APlayerController>(CurrentController);
			FVector WorldLocation, WorldDirection;
			if (PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
			{
				FHitResult HitResult;
				FVector StartTrace = WorldLocation;
				FVector EndTrace = WorldLocation + WorldDirection * CurrentWeapon->MaxRange;

				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				bool bHit = GetWorld()->LineTraceSingleByChannel(
					HitResult,
					StartTrace,
					EndTrace,
					ECollisionChannel::ECC_Visibility,
					Params
				);

				TargetLocation = bHit ? HitResult.Location : EndTrace;
			}
		}
	}
	// 2. AI 컨트롤러 조준 로직
	else if (CurrentController->IsA<AAIController>())
	{
		AAIController* AIController = Cast<AAIController>(CurrentController);
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();

		// [핵심] 블랙보드에서 Player Vector Location 키의 위치를 가져와 타겟으로 설정합니다.
		TargetLocation = BlackboardComp->GetValueAsVector(TEXT("PlayerVectorLocation"));
	}
	else
	{
		return; // 다른 종류의 컨트롤러는 발사하지 않음
	}

	// TargetLocation이 유효하면 발사 방향 계산 (AI/플레이어 공통)
	if (TargetLocation != FVector::ZeroVector)
	{
		FireDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();
	}
	else // 유효한 타겟 위치가 없으면 정면으로 발사
	{
		FireDirection = GetActorForwardVector();
	}

	// 발사 전에 다시 한 번 구르기 상태 체크 (타이머 콜백에서 호출될 수 있으므로)
	// 이 체크는 매우 중요합니다 - 네트워크 지연으로 인해 구르기 중에 발사 요청이 도착할 수 있습니다
	if (Rolling)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_StartFire: Blocked - Character is rolling during fire sequence. Clearing fire timer."));
		// 발사 타이머가 돌고 있으면 즉시 중지
		if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(FireTimerHandle);
		}
		// 발사 정지 상태 확보
		Server_StopFire();
		return;
	}

	// 3. 발사 시도 (무기 클래스의 Fire 함수가 멀티캐스트를 처리함)
	// 발사 직전에 마지막으로 한 번 더 체크
	if (!Rolling)
	{
		CurrentWeapon->Fire(FireDirection);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_StartFire: Blocked - Character started rolling just before Fire() call."));
		if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(FireTimerHandle);
		}
		Server_StopFire();
		return;
	}

	// 4. 블루프린트 이벤트 호출 (추가 시각적 효과용)
	Multicast_StartFire();

	// 5. 연사 타이머 설정 (타이머가 돌고 있지 않을 때만 설정)
	// 구르기 중이 아닐 때만 타이머 설정
	if (!Rolling && !GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&ALostSectorCharacter::StartFire,
			CurrentWeapon->FireRate,
			true
		);
	}
	else if (Rolling)
	{
		// 구르기 중이면 타이머가 설정되지 않도록 보장
		if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(FireTimerHandle);
			UE_LOG(LogTemp, Warning, TEXT("Server_StartFire: Cleared fire timer because character is rolling."));
		}
	}
}

void ALostSectorCharacter::StopFire()
{
	// 클라이언트에서 서버로 RPC 호출
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_StopFire();
		return;
	}

	// 서버에서 실제 정지 로직 실행
	Server_StopFire();
}

bool ALostSectorCharacter::Server_StopFire_Validate()
{
	return true;
}

void ALostSectorCharacter::Server_StopFire_Implementation()
{
	if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}

	// 모든 클라이언트에서 발사 정지 이펙트/애니메이션 재생
	Multicast_StopFire();
}

bool ALostSectorCharacter::Server_UpdateRotation_Validate(FRotator NewRotation)
{
	return true;
}

void ALostSectorCharacter::Server_UpdateRotation_Implementation(FRotator NewRotation)
{
	ReplicatedRotation = NewRotation;
	SetActorRotation(NewRotation);
}

void ALostSectorCharacter::Move(const FInputActionValue& Value)
{
	// 구르기 중이면 이동 입력 무시
	if (Rolling)
	{
		return;
	}

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

void ALostSectorCharacter::Reload(const FInputActionValue& Value)
{
	// 구르기 중이면 재장전 불가 (최우선 체크)
	if (Rolling)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot reload: Character is rolling (Role: %d)."), (int32)GetLocalRole());
		return;
	}

	// 클라이언트에서 서버로 RPC 호출
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_Reload();
		return;
	}

	// 서버에서 실제 재장전 로직 실행
	Server_Reload();
}

bool ALostSectorCharacter::Server_Reload_Validate()
{
	// 구르기 중이면 검증 실패
	if (Rolling)
	{
		return false;
	}
	return true;
}

void ALostSectorCharacter::Server_Reload_Implementation()
{
	// 구르기 중이면 재장전 불가 (서버에서도 체크)
	if (Rolling)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_Reload: Blocked - Character is rolling."));
		return;
	}

	if (CurrentWeapon)
	{
		// 마지막 체크: 무기 재장전 직전에 다시 한 번 구르기 상태 확인
		if (Rolling)
		{
			UE_LOG(LogTemp, Warning, TEXT("Server_Reload: Blocked - Character started rolling just before reload."));
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("Server_Reload: Calling WeaponReload on %s (Role: %d)"), 
			*CurrentWeapon->GetName(), (int32)GetLocalRole());

		// AWeapon.cpp에서 구현된 WeaponReload 함수 호출
		// WeaponReload 내부에서 Multicast_PlayReloadAnimation이 호출됨
		CurrentWeapon->WeaponReload();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_Reload: No CurrentWeapon equipped."));
	}
}

void ALostSectorCharacter::Die()
{
	// 이미 죽었으면 중복 실행 방지
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	UE_LOG(LogTemp, Warning, TEXT("💀 Character %s has died!"), *GetName());

	TArray<FItemStack> DroppedItems; // ⬅️ 아이템을 저장할 임시 배열 선언

	// 서버에서만 실행
	if (HasAuthority())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());

		// 1. InventoryComponent 가져오기
		if (UInventoryComponent* InventoryComp = FindComponentByClass<UInventoryComponent>())
		{
			// 2. 인벤토리 데이터를 DroppedItems에 복사 (아이템 드롭)
			DroppedItems = InventoryComp->Slots;

			// 3. InventoryComponent의 인벤토리 초기화
			int32 RemovedCount = InventoryComp->Slots.Num();
			InventoryComp->Slots.Empty();
			InventoryComp->InitSlots(); // 빈 슬롯으로 초기화

			UE_LOG(LogTemp, Warning, TEXT("💀 Cleared %d items from InventoryComponent for drop."), RemovedCount);

			// 인벤토리 클리어 후 저장 (이 로직은 기존 코드를 유지)
			if (PC && PC->PlayerState)
			{
				FString PlayerID;
				if (PC->PlayerState->GetUniqueId().IsValid())
				{
					PlayerID = PC->PlayerState->GetUniqueId()->ToString();
				}
				else
				{
					PlayerID = FString::Printf(TEXT("Local_%d"), PC->PlayerState->GetPlayerId());
				}

				TArray<FItemStack> EmptyStorage;
				// UInventorySaveManager를 사용한다고 가정
				if (UInventorySaveManager::SavePlayerInventory(this, PlayerID,
					InventoryComp->Slots, EmptyStorage))
				{
					UE_LOG(LogTemp, Log, TEXT("💾 InventoryComponent cleared and saved on death: %s"), *PlayerID);
				}
			}
		}

		// 4. LootContainer 스폰 (드롭할 아이템이 있고 클래스가 지정되었을 때만)
		if (LootContainerClass && DroppedItems.Num() > 0)
		{
			FVector SpawnLocation = GetActorLocation();
			if (GetCapsuleComponent())
			{
				// 캡슐 컴포넌트의 절반 높이(Half Height)를 가져와서 Z축에서 뺍니다.
				float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				SpawnLocation.Z -= HalfHeight;

				// 상자가 땅속에 너무 깊이 박히는 것을 방지하기 위해 약간 다시 올릴 수도 있습니다 (선택적)
				// SpawnLocation.Z += 10.0f; 
			}

			FRotator SpawnRotation = GetActorRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Instigator = GetInstigator();

			ADeathDropBox* DeathBox = GetWorld()->SpawnActor<ADeathDropBox>(
				LootContainerClass,
				SpawnLocation,
				SpawnRotation,
				SpawnParams
			);

			if (DeathBox)
			{
				// 5. 컨테이너에 아이템 데이터 전달 및 초기화
				DeathBox->InitializeLoot(DroppedItems);
				UE_LOG(LogTemp, Log, TEXT("📦 Dropped DeathDropBox with %d unique stacks."), DroppedItems.Num());
			}
		}

		// 6. MyPlayerState의 인벤토리 초기화 (기존 로직 유지)
		if (PC && PC->PlayerState)
		{
			if (AMyPlayerState* MyPS = PC->GetPlayerState<AMyPlayerState>())
			{
				// 서버에서 직접 호출 (RPC가 자동으로 Implementation을 실행함)
				MyPS->Server_ClearInventoryOnDeath();
			}
		}
	}
	// 1. **입력 및 컨트롤 제거**
	if (AController* CharacterController = GetController())
	{
		// 컨트롤러에서 캐릭터 빙의 해제
		DetachFromControllerPendingDestroy();

		// **플레이어 컨트롤러**라면 입력 모드 변경
		if (APlayerController* PC = Cast<APlayerController>(CharacterController))
		{
			// 2. **UI 전용 입력 모드 설정**
			// 마우스 커서가 보이고, 게임 입력(이동, 발사)은 무시하며, UI만 상호작용 가능하게 합니다.
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(nullptr); // 특정 위젯에 포커스를 맞추지 않아도 됩니다.
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			PC->SetInputMode(InputMode);

			// 마우스 커서 보이게 설정
			PC->bShowMouseCursor = true;
			// DeathWidget은 Multicast_Die_Implementation()에서 모든 클라이언트에 대해 처리됩니다.
		}
	}

	if (CurrentWeapon)
	{
		// 무기를 월드에서 제거합니다.
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr; // 포인터를 비워줍니다.
		//UE_LOG(LogTemp, Log, TEXT("✅ Weapon %s destroyed on character death."), *CurrentWeapon->GetName());
	}
	// 3. **캐릭터 외형 및 물리 처리 (Ragdoll)**
	// GetMesh()는 서버/클라이언트 모두 복제된 데이터를 가지고 있습니다.
	if (GetMesh())
	{
		// 충돌 프로파일을 Ragdoll로 변경
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		// 물리 시뮬레이션 활성화 (Ragdoll)
		GetMesh()->SetSimulatePhysics(true);
	}

	// 캡슐 컴포넌트 충돌 비활성화 (Ragdoll이 캡슐에 걸리지 않도록)
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 4.액터 수명 설정
	// 5초 후 캐릭터 Actor 자체를 월드에서 제거합니다. (아이템 상자는 별개로 존재)
	SetLifeSpan(5.0f);

	// 모든 클라이언트에서 사망 이펙트/애니메이션 재생
	// 컨트롤러를 분리하기 전에 컨트롤러를 저장
	APlayerController* DeadPlayerController = Cast<APlayerController>(GetController());
	if (HasAuthority())
	{
		Multicast_Die(DeadPlayerController);
	}
}

// 멀티캐스트 함수 구현들
void ALostSectorCharacter::Multicast_StartFire_Implementation()
{
	// 모든 클라이언트에서 발사 이펙트/애니메이션 재생
	// 블루프린트에서 구현된 OnStartFire 이벤트 호출
	OnStartFire();
	OnAmmoUpdated();
	
	UE_LOG(LogTemp, Log, TEXT("Multicast_StartFire: %s"), *GetName());
}

void ALostSectorCharacter::Multicast_StopFire_Implementation()
{
	// 모든 클라이언트에서 발사 정지 이펙트/애니메이션 재생
	// 블루프린트에서 구현된 OnStopFire 이벤트 호출
	OnStopFire();
	
	UE_LOG(LogTemp, Log, TEXT("Multicast_StopFire: %s"), *GetName());
}

void ALostSectorCharacter::Multicast_EquipWeapon_Implementation()
{
	// 모든 클라이언트에서 무기 장착 시각적 효과 재생
	// 블루프린트에서 구현된 OnEquipWeapon 이벤트 호출
	OnEquipWeapon();
	
	UE_LOG(LogTemp, Log, TEXT("Multicast_EquipWeapon: %s"), *GetName());
}

void ALostSectorCharacter::Multicast_Die_Implementation(APlayerController* DeadPlayerController)
{
	// 모든 클라이언트에서 사망 이펙트/애니메이션 재생
	// 블루프린트에서 구현된 OnDie 이벤트 호출
	OnDie();
	
	UE_LOG(LogTemp, Log, TEXT("Multicast_Die: %s, IsPlayerControlled: %d"), *GetName(), IsPlayerControlled() ? 1 : 0);

	// 플레이어가 아닌 경우(몬스터/AI) UI를 표시하지 않음
	if (!IsPlayerControlled())
	{
		UE_LOG(LogTemp, Log, TEXT("💀 Multicast_Die: AI 캐릭터 사망 - UI 표시하지 않음"));
		return;
	}

	// 모든 클라이언트에서 게임 오버 위젯 표시
	// 서버 플레이어도 포함하여 자신의 캐릭터가 죽었을 때 UI를 표시
	APlayerController* PC = DeadPlayerController;
	
	// 파라미터로 받은 컨트롤러가 없으면 다른 방법으로 찾기
	if (!PC)
	{
		PC = Cast<APlayerController>(GetController());
	}
	
	// 여전히 없으면 서버 플레이어의 경우 GetFirstPlayerController() 사용
	if (!PC)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			ENetMode NetMode = GetNetMode();
			if (NetMode == NM_ListenServer || NetMode == NM_Standalone)
			{
				PC = World->GetFirstPlayerController();
				UE_LOG(LogTemp, Log, TEXT("💀 Multicast_Die: 서버 플레이어 컨트롤러 찾기 시도. NetMode: %d"), (int32)NetMode);
			}
		}
	}
	
	if (PC)
	{
		// IsLocalController()는 클라이언트에서만 true를 반환하므로,
		// 서버 플레이어의 경우 GetNetMode()를 확인하여 처리
		bool bShouldShowWidget = false;
		ENetMode NetMode = GetNetMode();
		
		if (PC->IsLocalController())
		{
			// 클라이언트 플레이어
			bShouldShowWidget = true;
			UE_LOG(LogTemp, Log, TEXT("💀 Multicast_Die: 클라이언트 플레이어 - UI 표시"));
		}
		else if (NetMode == NM_ListenServer || NetMode == NM_Standalone)
		{
			// 서버 플레이어 (Listen Server 또는 Standalone)
			// 파라미터로 받은 컨트롤러가 있으면 서버 플레이어
			// 또는 GetFirstPlayerController()로 가져온 경우 항상 서버 플레이어
			if (DeadPlayerController || !GetController())
			{
				bShouldShowWidget = true;
				UE_LOG(LogTemp, Log, TEXT("💀 Multicast_Die: 서버 플레이어 - UI 표시. HasController: %d, DeadPlayerController: %d"), 
					GetController() ? 1 : 0, DeadPlayerController ? 1 : 0);
			}
		}
		
		if (DeathWidgetClass && bShouldShowWidget)
		{
			if (!DeathWidgetInstance)
			{
				// 위젯 생성
				DeathWidgetInstance = CreateWidget<UUserWidget>(PC, DeathWidgetClass);

				if (DeathWidgetInstance)
				{
					// 뷰포트에 추가
					DeathWidgetInstance->AddToViewport();
					UE_LOG(LogTemp, Log, TEXT("💀 Death Widget added to viewport. NetMode: %d, IsLocalController: %d, HasController: %d"), 
						(int32)NetMode, PC->IsLocalController() ? 1 : 0, GetController() ? 1 : 0);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("💀 Death Widget 생성 실패!"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("💀 Death Widget이 이미 존재합니다."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("💀 Death Widget 표시 조건 불만족. DeathWidgetClass: %d, bShouldShowWidget: %d"), 
				DeathWidgetClass ? 1 : 0, bShouldShowWidget ? 1 : 0);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("💀 Multicast_Die: PlayerController를 찾을 수 없습니다. NetMode: %d"), (int32)GetNetMode());
	}
}

void ALostSectorCharacter::InitializeMiniMap()
{
	if (!IsPlayerControlled())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 미니맵 Scene Capture 액터 생성
	if (!MiniMapCaptureActor)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		
		MiniMapCaptureActor = World->SpawnActor<AMiniMapCapture>(
			AMiniMapCapture::StaticClass(),
			GetActorLocation() + FVector(0.0f, 0.0f, 5000.0f),
			FRotator(-90.0f, 0.0f, 0.0f),
			SpawnParams
		);

		if (MiniMapCaptureActor)
		{
			MiniMapCaptureActor->SetTarget(this);
			UE_LOG(LogTemp, Log, TEXT("✅ MiniMap Capture Actor 생성 완료"));
		}
	}

	// 미니맵 위젯 생성
	if (MiniMapWidgetClass && !MiniMapWidgetInstance)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			MiniMapWidgetInstance = CreateWidget<UUserWidget>(PC, MiniMapWidgetClass);
			if (MiniMapWidgetInstance)
			{
				MiniMapWidgetInstance->AddToViewport();
				
				// Render Target을 위젯에 설정
				if (UMiniMapWidget* MiniMapWidget = Cast<UMiniMapWidget>(MiniMapWidgetInstance))
				{
					if (MiniMapCaptureActor && MiniMapCaptureActor->GetRenderTarget())
					{
						// Material을 사용하여 Render Target을 표시
						// 블루프린트에서 Material Instance를 설정하거나,
						// 여기서 Material을 로드하여 설정할 수 있습니다.
						UE_LOG(LogTemp, Log, TEXT("✅ MiniMap Widget 생성 완료. Render Target: %s"), 
							*MiniMapCaptureActor->GetRenderTarget()->GetName());
					}
				}
			}
		}
	}
}

void ALostSectorCharacter::UpdateMiniMap()
{
	if (!IsPlayerControlled() || !MiniMapWidgetInstance)
	{
		return;
	}

	if (UMiniMapWidget* MiniMapWidget = Cast<UMiniMapWidget>(MiniMapWidgetInstance))
	{
		// 플레이어 위치 업데이트
		FVector Location = GetActorLocation();
		FRotator Rotation = GetActorRotation();
		
		FVector2D WorldPosition = FVector2D(Location.X, Location.Y);
		float YawRotation = Rotation.Yaw;
		
		MiniMapWidget->UpdatePlayerPosition(WorldPosition, YawRotation);
	}
}

// 구르기 관련 함수 구현
void ALostSectorCharacter::PlayRollAnimation(UAnimMontage* RollMontage, float PlayRate)
{
	// 사망 상태면 구르기 불가
	if (bIsDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayRollAnimation: Character is dead, cannot roll."));
		return;
	}

	// 이미 구르기 중이면 완전히 차단 (스페이스바 입력 무시)
	if (Rolling)
	{
		// 타이머가 활성화되어 있는지 확인 (타이머가 없으면 상태 오류일 수 있음)
		if (!GetWorldTimerManager().IsTimerActive(RollingTimerHandle))
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayRollAnimation: Rolling is true but timer is not active! Resetting state."));
			Rolling = false;
			// 상태를 리셋했으므로 계속 진행
		}
		else
		{
			// 구르기 중이므로 완전히 차단
			UE_LOG(LogTemp, VeryVerbose, TEXT("PlayRollAnimation: Already rolling, completely blocking input."));
			return;
		}
	}

	// 스태미나 체크
	if (CharacterStats.Stamina < RollingStaminaCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayRollAnimation: Not enough stamina. Required: %f, Current: %f"), 
			RollingStaminaCost, CharacterStats.Stamina);
		return;
	}

	// 파라미터가 없으면 클래스 변수 사용
	if (!RollMontage)
	{
		RollMontage = RollingAnimMontage;
	}

	if (!RollMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayRollAnimation: No RollMontage specified! Set RollingAnimMontage in Blueprint."));
		return;
	}

	// PlayRate가 0 이하면 클래스 변수 사용
	if (PlayRate <= 0.0f)
	{
		PlayRate = RollingAnimPlayRate;
	}

	// 클라이언트에서 중복 호출 방지
	if (GetLocalRole() < ROLE_Authority)
	{
		// 이미 구르기 중이면 서버로 RPC를 보내지 않음
		// (네트워크 복제 지연으로 인한 중복 방지를 위해 로컬 상태도 확인)
		if (Rolling)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayRollAnimation: Client already rolling, preventing duplicate RPC."));
			return;
		}
		
		Server_PlayRollAnimation(RollMontage, PlayRate); // 서버로 RPC 호출
	}
	else // 서버라면 (또는 싱글 플레이어)
	{
		Server_PlayRollAnimation(RollMontage, PlayRate); // 서버에서 직접 실행
	}
}

bool ALostSectorCharacter::Server_PlayRollAnimation_Validate(UAnimMontage* RollMontage, float PlayRate)
{
	// 구르기 중이면 검증 실패
	if (Rolling)
	{
		return false;
	}
	
	return RollMontage != nullptr && PlayRate > 0.0f;
}

void ALostSectorCharacter::Server_PlayRollAnimation_Implementation(UAnimMontage* RollMontage, float PlayRate)
{
	if (!RollMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_PlayRollAnimation: RollMontage is null!"));
		return;
	}

	// 이미 구르기 중이면 중복 실행 방지
	if (Rolling)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_PlayRollAnimation: Already rolling, ignoring request."));
		return;
	}

	// 스태미나 소비
	if (!ConsumeStamina(RollingStaminaCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_PlayRollAnimation: Failed to consume stamina."));
		return;
	}

	// 구르기 상태를 먼저 설정 (발사 체크가 즉시 작동하도록)
	Rolling = true;
	
	// 구르기 시작 시 발사 완전히 중지 (구르기 중 총쏘기 방지)
	// 1. 발사 타이머 즉시 중지
	if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("Server_PlayRollAnimation: Stopped fire timer for rolling."));
	}
	
	// 2. 발사 정지 RPC 호출 (모든 클라이언트에서 발사 정지)
	Server_StopFire();
	
	// 3. 추가 안전장치: 발사가 진행 중이면 즉시 중지
	// (서버 플레이어의 경우 직접 호출될 수 있으므로)
	if (HasAuthority() && GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("Server_PlayRollAnimation: Force cleared fire timer (server player)."));
	}
	
	UE_LOG(LogTemp, Log, TEXT("Server_PlayRollAnimation: Starting roll animation. Stamina remaining: %f"), CharacterStats.Stamina);
	
	// 모든 클라이언트에서 애니메이션 재생 (파라미터 전달)
	Multicast_PlayRollingAnimation(RollMontage, PlayRate);

	// 구르기 종료 타이머 설정
	GetWorldTimerManager().SetTimer(
		RollingTimerHandle,
		this,
		&ALostSectorCharacter::OnRollingEnd,
		RollingDuration,
		false
	);
}

void ALostSectorCharacter::Multicast_PlayRollingAnimation_Implementation(UAnimMontage* RollMontage, float PlayRate)
{
	bool bAnimationPlayed = false;
	
	if (RollMontage && GetMesh())
	{
		float MontageLength = PlayAnimMontage(RollMontage, PlayRate);
		if (MontageLength > 0.0f)
		{
			bAnimationPlayed = true;
			UE_LOG(LogTemp, Log, TEXT("Multicast_PlayRollingAnimation: Playing roll animation. Length: %f"), MontageLength);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Multicast_PlayRollingAnimation: Failed to play animation montage!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Multicast_PlayRollingAnimation: Cannot play animation - RollMontage: %s, Mesh: %s"), 
			RollMontage ? TEXT("Valid") : TEXT("Null"), GetMesh() ? TEXT("Valid") : TEXT("Null"));
	}
	
	// 애니메이션 재생 실패 시 서버에서 상태 복구
	if (!bAnimationPlayed && HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Multicast_PlayRollingAnimation: Animation failed to play, resetting roll state."));
		Rolling = false;
		// 스태미나 복구 (선택적 - 실패 시 복구할지 결정)
		// CharacterStats.Stamina += RollingStaminaCost;
		// CharacterStats.Stamina = FMath::Clamp(CharacterStats.Stamina, 0.0f, 100.0f);
		return;
	}
	
	// 블루프린트에서 구현된 OnRollingAnimation 이벤트 호출
	OnRollingAnimation();
	
	// 구르기 시작 시 총소기 해제를 위한 이벤트 호출
	OnRollingStart();
}

void ALostSectorCharacter::PlayRoll()
{
	// 구르기 중이면 완전히 차단
	if (Rolling)
	{
		return;
	}

	// 클래스 변수를 사용하여 구르기 실행
	PlayRollAnimation(RollingAnimMontage, RollingAnimPlayRate);
}

void ALostSectorCharacter::ResetRollingState()
{
	if (Rolling)
	{
		UE_LOG(LogTemp, Warning, TEXT("ResetRollingState: Forcing roll state to false."));
		Rolling = false;
		GetWorldTimerManager().ClearTimer(RollingTimerHandle);
	}
}

void ALostSectorCharacter::OnRollingEnd()
{
	Rolling = false;
	GetWorldTimerManager().ClearTimer(RollingTimerHandle);
	
	UE_LOG(LogTemp, Log, TEXT("OnRollingEnd: Roll finished."));
	
	// 구르기 종료 시 발사 타이머가 돌고 있으면 중지
	// (구르기 중에 발사 요청이 들어와서 큐에 쌓였을 수 있음)
	if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("OnRollingEnd: Cleared fire timer that was active after rolling."));
	}
	
	// 발사 정지 상태 확보 (구르기 종료 후 자동 발사 방지)
	Server_StopFire();
	
	// 구르기 종료 시 총소기 복원을 위한 이벤트 호출
	OnRollingFinished();
}

void ALostSectorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// [DOREPLIFETIME 표준 사용] CharacterStats 변수 전체를 복제 대상으로 등록
	DOREPLIFETIME(ALostSectorCharacter, CharacterStats);
	DOREPLIFETIME(ALostSectorCharacter, HeadPitch);
	DOREPLIFETIME(ALostSectorCharacter, ReplicatedRotation);
	DOREPLIFETIME(ALostSectorCharacter, Rolling);
	DOREPLIFETIME(ALostSectorCharacter, bIsSprinting);
}