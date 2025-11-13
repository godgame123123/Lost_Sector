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
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
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
	
	// 캐릭터 생성 시 기본 Stamina 값 설정
	CharacterStats.Stamina = 100;

}

void ALostSectorCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// BeginPlay에서 StaminaRegenDrainTick 함수를 0.1초마다 반복 호출하도록 타이머 설정
	GetWorldTimerManager().SetTimer(
		StaminaTimerHandle,
		this,
		&ALostSectorCharacter::StaminaRegenDrainTick,
		0.1f, // 틱 간격
		true  // 반복
	);
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
bool ALostSectorCharacter::ConsumeStamina(int32 StaminaCost)
{
	// 1. Stamina가 비용보다 크거나 같은지 확인
	if (CharacterStats.Stamina >= StaminaCost)
	{
		// 2. Stamina 감소
		CharacterStats.Stamina -= StaminaCost;
		// (선택) 디버깅 로그 출력
		UE_LOG(LogTemp, Warning, TEXT("Stamina Consumed: %d. Current Stamina: %d"), StaminaCost, CharacterStats.Stamina);
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
	const int32 MaxStamina = 100;
	int32 StaminaChange = 0;
	// 현재 월드 시간 가져오기
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	// Stamina 재생 지연 시간 (초)
	const float LastDrainTime = FMath::Max(LastSprintEndTime, LastStaminaZeroTime);
	// 재생 지연 시간 계산
	const float RegenAllowedTime = LastSprintEndTime + StaminaRegenDelayDuration;
	// 달리기 상태 체크 및 Stamina 변경량 결정
	if (bIsSprinting)
	{
		// 달릴 때: Stamina 소모 (예: 틱당 -1)
		StaminaChange = -1;
	}
	else // 평소 상태 (재생 시도)
	{
		// Stamina가 Max보다 작을 때만 재생 시도
		if (CharacterStats.Stamina < MaxStamina)
		{
			// 지연 시간이 지났는지 확인
			if (CurrentTime < RegenAllowedTime)
			{
				StaminaChange = 0; // 지연 시간 이내: 재생 막음
			}
			else
			{
				StaminaChange = 1; // 지연 시간 경과: Stamina 재생
			}
		}
	}
	// FCharacterData::Stamina 값 업데이트
	CharacterStats.Stamina += StaminaChange;

	// 최소/최대 값으로 Clamp (0 이하, Max 이상으로 넘어가지 않게 방지)
	CharacterStats.Stamina = FMath::Clamp(CharacterStats.Stamina, 0, MaxStamina);

	// Stamina가 0에 도달했을 때의 로직
	if (CharacterStats.Stamina <= 0)
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
		LastStaminaZeroTime = CurrentTime;
	}
	// Stamina가 1 이상으로 올라오면 (재생 가능 상태가 되면) LastStaminaZeroTime 초기화
	// (이렇게 해야 지연 시간이 끝난 후 재생이 시작됨과 동시에 LastStaminaZeroTime이 갱신되지 않습니다.)
	else
	{
		LastStaminaZeroTime = 0.0f; // Stamina가 0이 아닐 때는 초기화
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