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
	
	// ĳ���� ���� �� �⺻ Stats ��
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
}
float ALostSectorCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
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

	if (bIsSprinting)
	{
		if (MovementComp && !MovementComp->bOrientRotationToMovement)
		{
			MovementComp->bOrientRotationToMovement = true;
		}

		return;
	}
	else 
	{
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

		SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));

	}
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
	if (bIsSprinting == true && bNewState == false)
	{
		LastSprintEndTime = GetWorld()->GetTimeSeconds();
	}
	bIsSprinting = bNewState;
}
void ALostSectorCharacter::StaminaRegenDrainTick()
{
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
			if (GetCharacterMovement())
			{
				GetCharacterMovement()->MaxWalkSpeed = 500.0f;
			}
			bIsSprinting = false;
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

	USceneComponent* MuzzleComp = CurrentWeapon->GetMuzzleLocation();
	if (!MuzzleComp)
	{
		return;
	}
	FVector MuzzleLocation = MuzzleComp->GetComponentLocation();

	FVector FireDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();

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

void ALostSectorCharacter::Die()
{
	// 이미 죽었으면 중복 실행 방지
	if (bIsDead)
	{
		return;
	}
	
	bIsDead = true;
	
	UE_LOG(LogTemp, Warning, TEXT("💀 Player %s has died!"), *GetName());
	
	// 서버에서만 실행
	if (HasAuthority())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (!PC || !PC->PlayerState)
		{
			return;
		}
		
		FString PlayerID;
		if (PC->PlayerState->GetUniqueId().IsValid())
		{
			PlayerID = PC->PlayerState->GetUniqueId()->ToString();
		}
		else
		{
			PlayerID = FString::Printf(TEXT("Local_%d"), PC->PlayerState->GetPlayerId());
		}
		
		// 1. MyPlayerState의 인벤토리 초기화
		if (AMyPlayerState* MyPS = PC->GetPlayerState<AMyPlayerState>())
		{
			// 서버에서 직접 호출 - RPC를 호출하면 자동으로 Implementation이 실행됨
			MyPS->Server_ClearInventoryOnDeath();
		}
		
		// 2. InventoryComponent의 인벤토리 초기화
		if (UInventoryComponent* InventoryComp = FindComponentByClass<UInventoryComponent>())
		{
			int32 RemovedCount = InventoryComp->Slots.Num();
			InventoryComp->Slots.Empty();
			InventoryComp->InitSlots(); // 빈 슬롯으로 초기화
			
			UE_LOG(LogTemp, Warning, TEXT("💀 Cleared %d items from InventoryComponent"), RemovedCount);
			
			// InventoryComponent 데이터 저장
			TArray<FItemStack> EmptyStorage;
			if (UInventorySaveManager::SavePlayerInventory(this, PlayerID, 
				InventoryComp->Slots, EmptyStorage))
			{
				UE_LOG(LogTemp, Log, TEXT("💾 InventoryComponent cleared and saved on death: %s"), *PlayerID);
			}
		}
	}
}