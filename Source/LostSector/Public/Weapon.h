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

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
    TObjectPtr<UAnimMontage> ReloadAnimMontage;

    
public:

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    TObjectPtr<class UStaticMeshComponent> WeaponMesh;


    
    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    TObjectPtr<USceneComponent> MuzzleLocation;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    USceneComponent* GetMuzzleLocation() const { return MuzzleLocation; }

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Stats", ReplicatedUsing = OnRep_CurrentAmmo)
    int32 CurrentAmmo;

    UFUNCTION()
    void OnRep_CurrentAmmo();


    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Fire(FVector Direction);

    // 애니메이션을 모든 클라이언트에서 재생하는 Multicast RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayFireAnimation();

    // 발사 이펙트를 모든 클라이언트에서 재생하는 Multicast RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayFireEffects(FVector StartLocation, FVector Direction);

    // 재장전 애니메이션을 모든 클라이언트에서 재생하는 Multicast RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayReloadAnimation();

    // 발사 사운드를 모든 클라이언트에서 재생하는 Multicast RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayFireSound(FVector SoundLocation);

    // 재장전 사운드를 모든 클라이언트에서 재생하는 Multicast RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayReloadSound(FVector SoundLocation);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Customization")
    FVector MeshOffsetLocation; 

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Customization")
    FRotator MeshOffsetRotation; 

    
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnFireEvent();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnReloadEvent();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void WeaponReload();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    TObjectPtr<class UItemDataBase> RequiredAmmoItemData;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    FTimerHandle FireRateTimerHandle;

    FTimerHandle ReloadTimerHandle;
    bool bCanFire = true;

    float LastFireTime = 0.0f;

    bool bIsReloading = false;

    void FinishReload();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float ReloadDuration = 2.0f;

    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float SpreadResetDuration = 0.2f; 

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float ReloadNoiseRange = 1500.0f;

    virtual void BeginPlay() override;
    void ResetFire();

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    class USoundBase* FireSound;

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    class USoundBase* ReloadSound;

    
private:
    void PerformLineTrace(FVector Start, FVector Direction);
};
