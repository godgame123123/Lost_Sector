// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class LOSTSECTOR_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stats")
    float Damage = 10.0f;           // 기본 데미지

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stats")
    float MaxRange = 5000.0f;       // 최대 사거리 (라인 트레이스 길이)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stats")
    float FireRate = 0.1f;          // 연사 속도 (발사 딜레이, 초)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stats")
    int32 MaxAmmo = 30;             // 탄창 크기

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Stats")
	float NoiseRange;               // 소음 범위

    // 총기 외형 (Static Mesh)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    TObjectPtr<class UStaticMeshComponent> WeaponMesh;

    // 총구 위치를 잡기 위한 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    TObjectPtr<USceneComponent> MuzzleLocation;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    USceneComponent* GetMuzzleLocation() const { return MuzzleLocation; }

    // 현재 탄약 수는 게임 중 변하므로 Editable이 아닌 VisibleAnywhere로 설정
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
    int32 CurrentAmmo;

    // 발사 기능 (Input Action이나 AI 로직에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Fire(FVector Direction);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Customization")
    FVector MeshOffsetLocation; // 소켓 기준 상대 위치 이동

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Customization")
    FRotator MeshOffsetRotation; // 소켓 기준 상대 회전 보정

    // 총소리 및 이펙트 재생을 위해 블루프린트에서 구현할 이벤트 (Report Noise Event 연결용)
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnFireEvent();
protected:
    FTimerHandle FireRateTimerHandle;
    bool bCanFire = true;

    virtual void BeginPlay() override;
    void ResetFire();

private:
    void PerformLineTrace(FVector Start, FVector Direction);
};
