// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystem.h" 
#include "NiagaraSystem.h" 
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
    float Damage = 10.0f;           

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float MaxRange = 5000.0f;       

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float FireRate = 0.1f;          

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 MaxAmmo = 30;            

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float NoiseRange;               

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float SpreadAngle = 2.0f;       

protected:
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
    UNiagaraSystem* MuzzleFlashFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    TSubclassOf<AATracer> TracerActorClass; 

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
    UNiagaraSystem* HitImpactFX;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
    TObjectPtr<UAnimMontage> FireAnimMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    TObjectPtr<UItemDataBase> RequiredAmmoItemData;
public:

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    TObjectPtr<class UStaticMeshComponent> WeaponMesh;

    
    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    TObjectPtr<USceneComponent> MuzzleLocation;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    USceneComponent* GetMuzzleLocation() const { return MuzzleLocation; }

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 CurrentAmmo;

    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Fire(FVector Direction);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Customization")
    FVector MeshOffsetLocation; 

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Customization")
    FRotator MeshOffsetRotation; 

    
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnFireEvent();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void WeaponReload();
protected:
    FTimerHandle FireRateTimerHandle;
    bool bCanFire = true;

    float LastFireTime = 0.0f;

    bool bIsReloading = false;

    void FinishReload();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float ReloadDuration = 2.0f;

    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float SpreadResetDuration = 0.2f; 

    virtual void BeginPlay() override;
    void ResetFire();

private:
    void PerformLineTrace(FVector Start, FVector Direction);
};
