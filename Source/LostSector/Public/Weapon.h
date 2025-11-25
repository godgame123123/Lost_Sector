// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystem.h" // UParticleSystem ��� Fx.h�� ����ϴ� ��� ���̾ư��� ����� �ʿ��մϴ�.
#include "NiagaraSystem.h" // ���̾ư��� �ý����� ���� ���
#include "ItemDataBase.h"
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
    float Damage = 10.0f;           // �⺻ ������

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float MaxRange = 5000.0f;       // �ִ� ��Ÿ� (���� Ʈ���̽� ����)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float FireRate = 0.1f;          // ���� �ӵ� (�߻� ������, ��)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 MaxAmmo = 30;             // źâ ũ��

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float NoiseRange;               // ���� ����

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float SpreadAngle = 2.0f;       // �Ѿ� �л� ����

protected:
    // Muzzle Flash Niagara System (�ѱ� ȭ��)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
    UNiagaraSystem* MuzzleFlashFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    TSubclassOf<AATracer> TracerActorClass; // ��˱���

    // Hit Impact Niagara System (��Ʈ ����Ʈ)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
    UNiagaraSystem* HitImpactFX;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
    TObjectPtr<UAnimMontage> FireAnimMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    TObjectPtr<UItemDataBase> RequiredAmmoItemData;
public:

    // �ѱ� ���� (Static Mesh)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    TObjectPtr<class UStaticMeshComponent> WeaponMesh;

    // �ѱ� ��ġ�� ��� ���� ������Ʈ
    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    TObjectPtr<USceneComponent> MuzzleLocation;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    USceneComponent* GetMuzzleLocation() const { return MuzzleLocation; }

    // ���� ź�� ���� ���� �� ���ϹǷ� Editable�� �ƴ� VisibleAnywhere�� ����
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 CurrentAmmo;

    // �߻� ��� (Input Action�̳� AI �������� ȣ��)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Fire(FVector Direction);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Customization")
    FVector MeshOffsetLocation; // ���� ���� ��� ��ġ �̵�

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Customization")
    FRotator MeshOffsetRotation; // ���� ���� ��� ȸ�� ����

    // �ѼҸ� �� ����Ʈ ����� ���� �������Ʈ���� ������ �̺�Ʈ (Report Noise Event �����)
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnFireEvent();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void WeaponReload();
protected:
    FTimerHandle FireRateTimerHandle;
    bool bCanFire = true;

    // [�߰�] ������ �߻� �ð� ���
    float LastFireTime = 0.0f;

    // [�߰�] ���� �߻� Ÿ�̹� (�� �ð� �ȿ� �ٽ� ��� ����� ����)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float SpreadResetDuration = 0.2f; // FireRate���� �ణ ��� ���� (��: 0.1f)

    virtual void BeginPlay() override;
    void ResetFire();

private:
    void PerformLineTrace(FVector Start, FVector Direction);
};
