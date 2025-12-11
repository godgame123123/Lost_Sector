// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CharacterDataStructs.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimMontage.h"
#include "LostSectorCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ADeathDropBox;
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

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

public:
	ALostSectorCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Replicated)
	FCharacterData CharacterStats;

	UFUNCTION(BlueprintCallable, Category = "Stats|Movement")
	bool ConsumeStamina(float StaminaCost);
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_IsSprinting)
	bool bIsSprinting = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 500.0f;

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetIsSprinting(bool bNewState);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetIsSprinting(bool bNewState);

	UFUNCTION()
	void OnRep_IsSprinting();
	
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

	FTimerHandle RollingTimerHandle;

	UFUNCTION()
	void OnRollingEnd();

	void SetActorOpacity(UPrimitiveComponent* MeshComp, float TargetOpacity);
public:
	// 구르기 관련 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement", Replicated)
	bool Rolling = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	UAnimMontage* RollingAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float RollingAnimPlayRate = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float RollingDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float RollingStaminaCost = 20.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<class AWeapon> CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> DefaultWeaponClass;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipWeapon();

	// 블루프린트에서 구현 가능한 무기 장착 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnEquipWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartFire(FVector ClientTargetLocation = FVector::ZeroVector);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartFire();

	// 블루프린트에서 구현 가능한 발사 시작 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnStartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StopFire();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopFire();

	// 블루프린트에서 구현 가능한 발사 정지 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnStopFire();
public:

	// 플레이어 사망 처리
	UFUNCTION(BlueprintCallable, Category = "Death")
	void Die();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Die(APlayerController* DeadPlayerController);

	// 블루프린트에서 구현 가능한 사망 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Death")
	void OnDie();

	// 구르기 관련 함수
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void PlayRollAnimation(UAnimMontage* RollMontage = nullptr, float PlayRate = 0.0f);
	
	// 파라미터 없이 호출 가능한 구르기 함수 (블루프린트 편의용)
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void PlayRoll();

	// 구르기 상태 강제 리셋 (디버그/복구용)
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ResetRollingState();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PlayRollAnimation(UAnimMontage* RollMontage, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayRollingAnimation(UAnimMontage* RollMontage, float PlayRate);

	// 블루프린트에서 구현 가능한 구르기 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void OnRollingAnimation();

	// 블루프린트에서 구현 가능한 구르기 시작 이벤트 (구르기 시 총소기 해제용)
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void OnRollingStart();

	// 블루프린트에서 구현 가능한 구르기 종료 이벤트 (구르기 종료 시 총소기 복원용)
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void OnRollingFinished();

protected:
	// ADeathDropBox 클래스 사용을 위한 전방 선언
	//class ADeathDropBox;

	// 스폰할 사망 드롭 박스 블루프린트 클래스를 지정합니다.
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	TSubclassOf<ADeathDropBox> LootContainerClass;
protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void Reload(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reload();

	bool Server_Reload_Validate();
	void Server_Reload_Implementation();
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

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> DeathWidgetClass;

	// 생성된 위젯 인스턴스 (메모리 관리용)
	UPROPERTY()
	TObjectPtr<class UUserWidget> DeathWidgetInstance;

	UPROPERTY()
	TSet<AActor*> ActorsToRestoreOpacity;

	// [추가] 투명화 처리 로직을 매 프레임 실행할 함수 선언
	void HandleOcclusionFade();
public:
	// 재장전 텍스트 위젯의 표시 여부를 제어하는 공용 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetReloadingTextVisible(bool bShow); // 함수명 변경 (ShowReloadingText 대신 SetReloadingTextVisible 사용)

	// 재장전 텍스트를 모든 클라이언트에서 표시/숨김하는 Multicast RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetReloadingTextVisible(bool bShow);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Look", Replicated)
	float HeadPitch = 0.0f; // ⬅️ 이 변수를 추가합니다.

	UPROPERTY(Replicated)
	FRotator ReplicatedRotation;

	// 회전 정보를 서버로 전송하는 RPC
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UpdateRotation(FRotator NewRotation);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};