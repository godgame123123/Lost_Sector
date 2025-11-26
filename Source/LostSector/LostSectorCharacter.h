// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CharacterDataStructs.h"
#include "Components/WidgetComponent.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

public:
	ALostSectorCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated)
	FCharacterData CharacterStats;

	UFUNCTION(BlueprintCallable, Category = "Stats|Movement")
	bool ConsumeStamina(float StaminaCost);
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	bool bIsSprinting = false;

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetIsSprinting(bool bNewState);
	
private:
	FTimerHandle StaminaTimerHandle;
	UFUNCTION()
	void StaminaRegenDrainTick();

	float LastSprintEndTime;

	float LastStaminaZeroTime;

	const float StaminaRegenDelayDuration = 2.0f;

	FTimerHandle HungerTimerHandle;
	UFUNCTION()
	void HungerDrainTick();

	// 사망 상태 플래그
	bool bIsDead = false;

	const float HungerDrainPerTick = 0.05f;

	const int32 HealthDrainPerTick = 1;

	FTimerHandle FireTimerHandle;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<class AWeapon> CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> DefaultWeaponClass;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();
public:

	// 플레이어 사망 처리
	UFUNCTION(BlueprintCallable, Category = "Death")
	void Die();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void Reload(const FInputActionValue& Value);
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

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:
	// 텍스트 위젯을 캐릭터 머리 위에 띄우기 위한 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> ReloadTextWidgetComponent;

public:
	// 재장전 텍스트 위젯의 표시 여부를 제어하는 공용 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetReloadingTextVisible(bool bShow); // 함수명 변경 (ShowReloadingText 대신 SetReloadingTextVisible 사용)
};