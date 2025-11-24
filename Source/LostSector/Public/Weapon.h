// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystem.h" // UParticleSystem 대신 Fx.h를 사용하는 경우 나이아가라 헤더가 필요합니다.
#include "NiagaraSystem.h" // 나이아가라 시스템을 위한 헤더
#include "Weapon.generated.h"

class AATracer;
class UAnimMontage;
UCLASS()
class LOSTSECTOR_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float Damage = 10.0f;           // 기본 데미지

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float MaxRange = 5000.0f;       // 최대 사거리 (라인 트레이스 길이)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float FireRate = 0.1f;          // 연사 속도 (발사 딜레이, 초)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 MaxAmmo = 30;             // 탄창 크기

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float NoiseRange;               // 소음 범위

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float SpreadAngle = 2.0f;       // 총알 분산 각도

protected:
    // Muzzle Flash Niagara System (총구 화염)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
    UNiagaraSystem* MuzzleFlashFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    TSubclassOf<AATracer> TracerActorClass; // 충알궤적

    // Hit Impact Niagara System (히트 임팩트)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
    UNiagaraSystem* HitImpactFX;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
    TObjectPtr<UAnimMontage> FireAnimMontage;
public:

    // 총기 외형 (Static Mesh)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    TObjectPtr<class UStaticMeshComponent> WeaponMesh;

    // 총구 위치를 잡기 위한 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    TObjectPtr<USceneComponent> MuzzleLocation;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    USceneComponent* GetMuzzleLocation() const { return MuzzleLocation; }

    // 현재 탄약 수는 게임 중 변하므로 Editable이 아닌 VisibleAnywhere로 설정
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 CurrentAmmo;

    // 발사 기능 (Input Action이나 AI 로직에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Fire(FVector Direction);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Customization")
    FVector MeshOffsetLocation; // 소켓 기준 상대 위치 이동

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Customization")
    FRotator MeshOffsetRotation; // 소켓 기준 상대 회전 보정

    // 총소리 및 이펙트 재생을 위해 블루프린트에서 구현할 이벤트 (Report Noise Event 연결용)
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnFireEvent();
protected:
    FTimerHandle FireRateTimerHandle;
    bool bCanFire = true;

    // [추가] 마지막 발사 시간 기록
    float LastFireTime = 0.0f;

    // [추가] 연속 발사 타이밍 (이 시간 안에 다시 쏘면 연사로 간주)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float SpreadResetDuration = 0.2f; // FireRate보다 약간 길게 설정 (예: 0.1f)

    virtual void BeginPlay() override;
    void ResetFire();

private:
    void PerformLineTrace(FVector Start, FVector Direction);
};
