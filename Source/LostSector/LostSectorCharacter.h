// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CharacterDataStructs.h"
#include "LostSectorCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class ALostSectorCharacter : public ACharacter
{
	GENERATED_BODY()
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
public:
	ALostSectorCharacter();

	// 1. Stamina 데이터를 Blueprint에서도 접근 가능하게 선언
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FCharacterData CharacterStats;

	// 2. Stamina를 감소시키는 함수를 선언하고 BlueprintCallable로 노출
	UFUNCTION(BlueprintCallable, Category = "Stats|Movement")
	bool ConsumeStamina(int32 StaminaCost);
public:
	// Blueprint에서 달리기 상태를 설정할 수 있도록 노출
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	bool bIsSprinting = false;

	// Blueprint에서 호출할 함수 (Left Shift Pressed/Released에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetIsSprinting(bool bNewState);
private:
	FTimerHandle StaminaTimerHandle;
	UFUNCTION()
	void StaminaRegenDrainTick(); // 0.1초마다 호출될 함수

	// 마지막 달리기 종료 시간을 기록할 변수 추가
	float LastSprintEndTime;

	// 스테미나가 0에 도달했을 때의 시간을 기록할 변수 추가
	float LastStaminaZeroTime;

	// 재생 지연 시간 (상수)
	const float StaminaRegenDelayDuration = 2.0f; // 1.0초 지연 (원하는 값으로 설정)
protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};