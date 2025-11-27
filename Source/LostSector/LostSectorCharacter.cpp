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
	}
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
void ALostSectorCharacter::SetActorOpacity(UPrimitiveComponent* MeshComp, float TargetOpacity)
{
	if (!MeshComp) return;

	for (int32 i = 0; i < MeshComp->GetNumMaterials(); ++i)
	{
		UMaterialInterface* Material = MeshComp->GetMaterial(i);
		UMaterialInstanceDynamic* DynamicMat = Cast<UMaterialInstanceDynamic>(Material);

		if (!DynamicMat)
		{
			// Dynamic Instance가 없다면 새로 만들고 설정합니다.
			DynamicMat = MeshComp->CreateAndSetMaterialInstanceDynamic(i);
		}

		if (DynamicMat)
		{
			// 재질에 "Opacity"라는 Scalar Parameter가 있어야 합니다.
			DynamicMat->SetScalarParameterValue(FName("Opacity"), TargetOpacity);
		}
	}
}
void ALostSectorCharacter::HandleOcclusionFade()
{
	if (!CameraBoom) return; // CameraBoom이 없다면 종료

	FVector PlayerLocation = GetActorLocation();
	FVector CameraLocation = CameraBoom->GetComponentLocation();

	// Line Trace 설정
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // 캐릭터 자신 무시
	Params.bReturnPhysicalMaterial = false;

	// Line Trace 실행 (카메라에서 플레이어까지)
	bool bHit = GetWorld()->LineTraceMultiByChannel(
		HitResults,
		CameraLocation,
		PlayerLocation,
		ECollisionChannel::ECC_Visibility, // Visibility 채널을 사용
		Params
	);

	TArray<AActor*> CurrentOccludingActors;

	// 1. 현재 트레이스에 걸린 액터 투명화
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		// Static/World Dynamic 액터만 처리합니다. (플레이어나 무기 같은 액터는 제외)
		if (HitActor && !HitActor->IsA<ACharacter>() && HitActor->GetRootComponent() && HitActor->GetRootComponent()->Mobility == EComponentMobility::Static)
		{
			CurrentOccludingActors.Add(HitActor);

			// 2. 액터의 모든 Primitive Component를 투명화 처리
			TArray<UPrimitiveComponent*> PrimitiveComponents;
			HitActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

			for (UPrimitiveComponent* Component : PrimitiveComponents)
			{
				// Opacity 0.3으로 설정 (반투명)
				SetActorOpacity(Component, 0.3f);
			}

			// 복구 목록에서 제거 (현재 투명 상태를 유지해야 함)
			ActorsToRestoreOpacity.Remove(HitActor);
		}
	}

	// 3. 이전 프레임에 투명화되었으나 현재는 걸리지 않은 액터 불투명으로 복구
	for (AActor* ActorToRestore : ActorsToRestoreOpacity)
	{
		if (ActorToRestore)
		{
			TArray<UPrimitiveComponent*> PrimitiveComponents;
			ActorToRestore->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

			for (UPrimitiveComponent* Component : PrimitiveComponents)
			{
				// Opacity 1.0으로 설정 (불투명)
				SetActorOpacity(Component, 1.0f);
			}
		}
	}

	// 4. 복구 목록 갱신
	ActorsToRestoreOpacity = CurrentOccludingActors;
}
void ALostSectorCharacter::SetReloadingTextVisible(bool bShow)
{
	if (ReloadTextWidgetComponent)
	{
		ReloadTextWidgetComponent->SetVisibility(bShow);
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
	}
}

void ALostSectorCharacter::StartFire()
{
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
		APlayerController* PC = CastChecked<APlayerController>(CurrentController); // 캐스팅 체크는 안정성을 높입니다.

		// 1. 마우스 위치를 월드 좌표로 변환합니다.
		FVector WorldLocation, WorldDirection;
		if (!PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			return; // 디프로젝트 실패 시 종료
		}

		// 2. 라인 트레이스 실행
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

		// 5. TargetLocation 결정
		TargetLocation = bHit ? HitResult.Location : EndTrace;
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

	// 3. 발사 시도
	CurrentWeapon->Fire(FireDirection);

	// 4. 연사 타이머 설정 (타이머가 돌고 있지 않을 때만 설정)
	if (!GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&ALostSectorCharacter::StartFire,
			CurrentWeapon->FireRate,
			true
		);
	}
}

void ALostSectorCharacter::StopFire()
{
	if (GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}
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

void ALostSectorCharacter::Reload(const FInputActionValue& Value)
{
	if (CurrentWeapon)
	{
		UE_LOG(LogTemp, Log, TEXT("Reload key pressed. Calling WeaponReload on %s"), *CurrentWeapon->GetName());

		// AWeapon.cpp에서 구현된 WeaponReload 함수 호출
		CurrentWeapon->WeaponReload();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot reload: No CurrentWeapon equipped."));
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
			if (DeathWidgetClass && PC->IsLocalController())
			{
				if (!DeathWidgetInstance)
				{
					// 위젯 생성
					DeathWidgetInstance = CreateWidget<UUserWidget>(PC, DeathWidgetClass);

					if (DeathWidgetInstance)
					{
						// 뷰포트에 추가
						DeathWidgetInstance->AddToViewport();
						UE_LOG(LogTemp, Log, TEXT("💀 Death Widget added to viewport."));
					}
				}
			}
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
}

void ALostSectorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// [DOREPLIFETIME 표준 사용] CharacterStats 변수 전체를 복제 대상으로 등록
	DOREPLIFETIME(ALostSectorCharacter, CharacterStats);
}