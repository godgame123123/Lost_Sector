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

	// 1. Stamina �����͸� Blueprint������ ���� �����ϰ� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FCharacterData CharacterStats;

	// 2. Stamina�� ���ҽ�Ű�� �Լ��� �����ϰ� BlueprintCallable�� ����
	UFUNCTION(BlueprintCallable, Category = "Stats|Movement")
	bool ConsumeStamina(float StaminaCost);
public:
	// Blueprint���� �޸��� ���¸� ������ �� �ֵ��� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	bool bIsSprinting = false;

	// Blueprint���� ȣ���� �Լ� (Left Shift Pressed/Released���� ȣ��)
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetIsSprinting(bool bNewState);
	
private:
	FTimerHandle StaminaTimerHandle;
	UFUNCTION()
	void StaminaRegenDrainTick(); // 0.1�ʸ��� ȣ��� �Լ�

	// ������ �޸��� ���� �ð��� ����� ���� �߰�
	float LastSprintEndTime;

	// ���׹̳��� 0�� �������� ���� �ð��� ����� ���� �߰�
	float LastStaminaZeroTime;

	// ��� ���� �ð� (���)
	const float StaminaRegenDelayDuration = 2.0f; // 1.0�� ���� (���ϴ� ������ ����)

	// �����/ü�� ���� �߰� ����
	FTimerHandle HungerTimerHandle; // ����� ƽ�� ���� Ÿ�̸� �ڵ�
	UFUNCTION()
	void HungerDrainTick(); // �ֱ������� ����Ŀ� ü�¸� üũ�ϰ� �Ҹ��� �Լ�

	// 사망 상태 플래그
	bool bIsDead = false;



	// ����� ��� ����
	// ����� �Ҹ� �ӵ�: 10�ʿ� 5�� ���� (0.1ƽ�� 0.05)
	const float HungerDrainPerTick = 0.05f;
	// ����� 0�� �� ü�� ���� �ӵ�: 1�ʴ� 10�� ���� (0.1ƽ�� 1)
	const int32 HealthDrainPerTick = 1;
	// �����/ü�� ���� �߰� �� 
public:
	// ���� ������ ���� ���͸� ������ ������
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<class AWeapon> CurrentWeapon;

	// ��������Ʈ���� � ���⸦ �������� ������ �� �ֵ��� UPROPERTY ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> DefaultWeaponClass;

	// ���⸦ �����ϰ� ĳ���Ϳ��� �����ϴ� �Լ�
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon();

	// ���� ������ Fire() �Լ��� ȣ�� (Input �����)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

public:

	// 플레이어 사망 처리
	UFUNCTION(BlueprintCallable, Category = "Death")
	void Die();

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
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};