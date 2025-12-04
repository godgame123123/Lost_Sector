// AWeapon.cpp
#include "Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h" 
#include "EngineUtils.h"
#include "NiagaraFunctionLibrary.h" // ���̾ư��� �Լ� ���̺귯��
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h" // ���̷�Ż �޽� ������Ʈ ������ ����
#include "ATracer.h"
#include "Animation/AnimInstance.h"
#include "InventoryComponent.h"
#include "../LostSectorCharacter.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    // 리플리케이션 설정
    bReplicates = true;
    SetReplicateMovement(true);

    // ��Ʈ ������Ʈ ����
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // ����ƽ �޽� ������Ʈ ���� �� ��Ʈ�� ����
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(RootComponent);

    // �ѱ� ��ġ ������Ʈ ���� (�߻� ���� ����)
    MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
    MuzzleLocation->SetupAttachment(WeaponMesh.Get());

    NoiseRange = 5000.0f;
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    // BeginPlay ������ �޽� ���� ���� �����մϴ�.
    if (WeaponMesh)
    {
        // 1. ��� ��ġ (Location) �� ȸ�� (Rotation) ����
        WeaponMesh->SetRelativeLocation(MeshOffsetLocation);
        WeaponMesh->SetRelativeRotation(MeshOffsetRotation);

        // 2. �����ϵ� �����ؾ� �Ѵٸ� (�� ũ�Ⱑ �ٸ� ���)
        // WeaponMesh->SetRelativeScale3D(FVector(1.0f)); // �ʿ��� ������ �� ����
    }
}

void AWeapon::ResetFire()
{
    bCanFire = true;
}

void AWeapon::PerformLineTrace(FVector Start, FVector Direction)
{
    // [��Ʈ��ĵ ����]
    FVector End = Start + (Direction * MaxRange);
    FHitResult HitResult;

    // Trace Channel�� Visibility �Ǵ� Custom Trace Channel�� ����
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); // ���� ���ʹ� Ʈ���̽����� ����

    AController* InstigatorController = GetInstigatorController();
    AActor* InstigatorPawn = GetInstigator();

    if (InstigatorPawn)
    {
        Params.AddIgnoredActor(InstigatorPawn); // �߻���(Instigator Pawn)�� ���� ��Ͽ� �߰�

        // �߻��ڰ� AI Controller�� ��� �ް� �ִٸ� (��, AI�� ���� ���̶��)
        if (InstigatorController && InstigatorController->IsA<AAIController>())
        {
            UWorld* World = GetWorld();
            if (World)
            {
                // ���� ���� ��� Pawn (ĳ����)�� ��ȸ�մϴ�.
                for (TActorIterator<APawn> It(World); It; ++It)
                {
                    APawn* Pawn = *It;

                    // 1. Pawn�� ��ȿ�ϰ� 2. �÷��̾ �������� �ʴ� ��� (��, �ٸ� AI/NPC�� ���)
                    // (�߰������� �� üũ ������ ���� �� ������, ���⼭�� ������ AI/NPC�� ����)
                    if (Pawn && !Pawn->IsPlayerControlled())
                    {
                        // �ش� AI/NPC ĳ���͸� ���� Ʈ���̽� ���� ��Ͽ� �߰��մϴ�.
                        Params.AddIgnoredActor(Pawn);
                    }
                }
            }
        }
    }

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility, // �ʿ信 ���� ����
        Params
    );

    FVector TargetLocation = bHit ? HitResult.Location : End;

    if (TracerActorClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        // Ʈ���̼� ���͸� �ѱ� ��ġ(Start)�� ����
        AATracer* TracerActor = GetWorld()->SpawnActor<AATracer>(
            TracerActorClass,
            Start,
            Direction.Rotation(),
            SpawnParams
        );

        if (TracerActor)
        {
            // 2. [�̵� ����] TargetLocation�� �ӵ��� �����մϴ�.
            //     �� �Լ� ȣ���� �����ϸ� AATracer�� TargetLocation ������ �⺻�� (FVector::ZeroVector)���� �����ְ� �˴ϴ�.

            const float BulletSpeed = 20000.0f; // �ſ� ���� �ӵ��� ���� (����: cm/s)

            // AATracer::StartMoving �Լ� ȣ��
            TracerActor->StartMoving(TargetLocation, BulletSpeed);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn AATracer actor!"));
        }
    }
    

    // 2. ��Ʈ ����Ʈ ����Ʈ ���� (�¾��� ��쿡��)
    if (bHit && HitImpactFX) // AWeapon.h�� ����� UNiagaraSystem* HitImpactFX ���� ���
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            HitImpactFX,
            HitResult.Location,
            HitResult.ImpactNormal.Rotation(), // ���� ���� ���� �������� ȸ��
            FVector(1.0f),
            true,
            true
        );
    }

    //FColor LineColor = bHit ? FColor::Red : FColor::Green;

    // DrawDebugLine �Լ��� Kismet/KismetMathLibrary.h �� ���ǵǾ� �ֽ��ϴ�.
    // ���� �ڵ忡�� #include "Kismet/KismetMathLibrary.h" �� ���ԵǾ� �����Ƿ� �ٷ� ��� �����մϴ�.
    //DrawDebugLine(
    //    GetWorld(),
    //    Start,
    //    bHit ? HitResult.Location : End, // ��Ʈ������ ��Ʈ ��������, �ƴϸ� �ִ� ��Ÿ�����
    //    LineColor,
    //    false,      // bPersistentLines (���������� ����)
    //    5.0f,       // LifeTime (5�ʰ� ǥ��)
    //    0,          // DepthPriority
    //    3.0f        // Thickness (���� �β�)
    //);

    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();
        if (HitActor)
        {
            // ������ ����
            UGameplayStatics::ApplyDamage(
                HitActor,
                Damage,
                GetInstigatorController(), // �߻��� ĳ������ ��Ʈ�ѷ��� ������ �����ڷ� ����
                this,
                nullptr // DamageTypeClass
            );

            // TODO: �ǰ� ����Ʈ �� ���� ���� (HitResult.Location, HitResult.ImpactNormal ���)
        }
    }

    // TODO: �ѱ� ȭ�� (Muzzle Flash) �� ź�� ���� ����Ʈ ����
}

void AWeapon::Fire(FVector Direction)
{

    if (bIsReloading)
    {
        UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: Weapon is currently reloading."));
        return;
    }
    if (!bCanFire)
    {
        UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: bCanFire=false"));
        return;
    }

    // CurrentAmmo�� 0�̸� �κ��丮���� �Ѿ� Ȯ��
    if (CurrentAmmo <= 0)
    {
        // �κ��丮���� �Ѿ� ���� Ȯ��
        APawn* OwnerPawn = Cast<APawn>(GetOwner());
        if (OwnerPawn)
        {
            UInventoryComponent* InventoryComp = OwnerPawn->FindComponentByClass<UInventoryComponent>();
            if (InventoryComp && RequiredAmmoItemData)
            {
                int32 AmmoCount = InventoryComp->GetItemCountByItemData(RequiredAmmoItemData);
                if (AmmoCount <= 0)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: No ammo in inventory. CurrentAmmo=%d, InventoryAmmo=%d"),
                        CurrentAmmo, AmmoCount);
                    return;
                }
                else
                {
                    // �κ��丮�� �Ѿ��� ������ �ڵ����� ������ �õ�
                    UE_LOG(LogTemp, Log, TEXT("Auto-reload: Found %d ammo in inventory"), AmmoCount);
                    WeaponReload();
                    return; // ������ ���̹Ƿ� �߻� �Ұ�
                }
            }
            else if (!RequiredAmmoItemData)
            {
                UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: No RequiredAmmoItemData set"));
                return;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: CurrentAmmo=%d, No inventory or owner"), CurrentAmmo);
        return;
    }

    // ���� �߻� ���� ���
    if (false) // ���� üũ�� ������ ó�������Ƿ� ���⼭�� �׻� false
    {
        
        UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: bCanFire=%s, Ammo=%d"),
            bCanFire ? TEXT("True") : TEXT("False"), CurrentAmmo);
        return;
    }

    
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    const bool bIsFirstShot = (CurrentTime - LastFireTime > SpreadResetDuration);

    
    LastFireTime = CurrentTime;

    
    UE_LOG(LogTemp, Log, TEXT("Fire Start! Ammo Left: %d, First Shot: %s"), CurrentAmmo - 1, bIsFirstShot ? TEXT("True") : TEXT("False"));

    
    bCanFire = false;
    CurrentAmmo--;

    
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn && FireAnimMontage) 
    {
        
        USkeletalMeshComponent* CharacterMesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();

        if (CharacterMesh)
        {
            UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();

            
            if (AnimInstance)
            {
                AnimInstance->Montage_Play(FireAnimMontage, 1.0f);
            }
        }
    }

    
    FVector StartLocation = MuzzleLocation->GetComponentLocation();

    if (MuzzleLocation) 
    {
        if (MuzzleFlashFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                MuzzleFlashFX,                  
                MuzzleLocation,                 
                NAME_None,                      
                FVector::ZeroVector,            
                FRotator::ZeroRotator,         
                EAttachLocation::SnapToTarget,  
                true                            
            );
        }
    }

  
    FVector FinalFireDirection = Direction;

    
    if (!bIsFirstShot && SpreadAngle > 0.0f)
    {
        
        const FRotator CurrentRotator = Direction.Rotation();

       
        float RandomYaw = FMath::FRandRange(-SpreadAngle, SpreadAngle);
        float RandomPitch = FMath::FRandRange(-SpreadAngle, SpreadAngle);

        
        const FRotator SpreadRotator = CurrentRotator + FRotator(RandomPitch, RandomYaw, 0.0f);

        
        FinalFireDirection = UKismetMathLibrary::GetForwardVector(SpreadRotator);
    }
  

    PerformLineTrace(StartLocation, FinalFireDirection); 

    OnFireEvent();


    GetWorld()->GetTimerManager().SetTimer(
        FireRateTimerHandle,
        this,
        &AWeapon::ResetFire,
        FireRate,
        false
    );
}

void AWeapon::WeaponReload()
{
    if (bIsReloading)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Already reloading."), *GetName());
        return;
    }
    // 1. �̹� �ִ� ź���̶�� ������ ���ʿ�
    if (CurrentAmmo >= MaxAmmo)
    {
        UE_LOG(LogTemp, Log, TEXT("%s: Ammo is already full (%d/%d)."), *GetName(), CurrentAmmo, MaxAmmo);
        if (GetWorld()->GetTimerManager().IsTimerActive(ReloadTimerHandle))
        {
            GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
        }
        ALostSectorCharacter* Character = Cast<ALostSectorCharacter>(GetOwner());
        if (Character)
        {
            Character->SetReloadingTextVisible(false); // ������ ���ʿ� �� ��� ���� ����
        }
        return;
    }

    // 2. ������(Pawn)�� �κ��丮 ������Ʈ ã��
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn || !RequiredAmmoItemData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Reload Failed: Missing OwnerPawn or RequiredAmmoItemData."));
        return;
    }

    UInventoryComponent* InventoryComp = OwnerPawn->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Reload Failed: Owner has no UInventoryComponent."));
        return;
    }

    // 3. �ʿ��� ź�� �� ��� �� �κ��丮���� ��� ������ �Ѿ� �� Ȯ��
    int32 AmmoNeeded = MaxAmmo - CurrentAmmo;
    int32 AvailableAmmo = InventoryComp->GetItemCountByItemData(RequiredAmmoItemData);

    if (AvailableAmmo <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Reload Failed: No available ammo in inventory."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("%s: Starting reload for %f seconds..."), *GetName(), ReloadDuration);

    bIsReloading = true; // ������ �÷��� ����
    bCanFire = false;    // ������ �� �߻� ���� (Fire �Լ����� üũ)

    ALostSectorCharacter* Character = Cast<ALostSectorCharacter>(GetOwner());
    if (Character)
    {
        Character->SetReloadingTextVisible(true);
    }

    GetWorld()->GetTimerManager().SetTimer(
        ReloadTimerHandle, // ���� (Ÿ�̸� �ڵ��� ���� �δ� ���� �� �����ϴ�. ��: ReloadTimerHandle)
        this,
        &AWeapon::FinishReload,
        ReloadDuration, // AWeapon.h���� ������ �ð� ���
        false
    );
}

void AWeapon::FinishReload()
{
    ALostSectorCharacter* Character = Cast<ALostSectorCharacter>(GetOwner());
    if (Character)
    {
        Character->SetReloadingTextVisible(false);
    }

    // 1. �÷��� ����
    bIsReloading = false;

    // 2. �����ڿ� �κ��丮 ã�� (WeaponReload������ ����)
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn || !RequiredAmmoItemData) return;
    UInventoryComponent* InventoryComp = OwnerPawn->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComp) return;

    // 3. �ʿ��� ź�� �� ��� �� �κ��丮���� ������ ź�� ����
    int32 AmmoNeeded = MaxAmmo - CurrentAmmo;
    int32 AvailableAmmo = InventoryComp->GetItemCountByItemData(RequiredAmmoItemData);
    int32 AmmoToTake = FMath::Min(AmmoNeeded, AvailableAmmo);

    // 4. �κ��丮���� ź�� ���� ��û
    int32 RemovedCount = InventoryComp->RemoveItemByItemData(RequiredAmmoItemData, AmmoToTake);

    // 5. źâ ������Ʈ
    if (RemovedCount > 0)
    {
        CurrentAmmo += RemovedCount;
        UE_LOG(LogTemp, Log, TEXT("%s: RELOAD FINISHED. Added %d rounds. New Ammo: %d/%d."),
            *GetName(), RemovedCount, CurrentAmmo, MaxAmmo);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RELOAD FINISHED: No ammo taken from inventory. Current Ammo: %d"), CurrentAmmo);
    }

    // ������ �߿��� �߻簡 ���� �־����Ƿ�, Ȥ�� �� ��Ȳ�� ����� Fire ���¸� �缳���մϴ�.
    ResetFire();
}
