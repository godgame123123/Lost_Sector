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
#include "Perception/AIPerceptionSystem.h"

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
    // 구르기 중이면 발사 불가 (최우선 체크)
    APawn* WeaponOwnerPawn = Cast<APawn>(GetOwner());
    if (WeaponOwnerPawn)
    {
        // ALostSectorCharacter로 캐스팅하여 구르기 상태 확인
        if (ALostSectorCharacter* Character = Cast<ALostSectorCharacter>(WeaponOwnerPawn))
        {
            // Rolling은 public이지만, 접근 가능한지 확인
            // LostSectorCharacter.h에서 Rolling이 public으로 선언되어 있는지 확인 필요
            // 일단 GetOwner()를 통해 접근하는 방식으로 변경
            if (Character && Character->Rolling)
            {
                UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: Character is rolling."));
                return;
            }
        }
    }

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

    // 서버에서만 멀티캐스트 호출 (모든 클라이언트에서 애니메이션 재생)
    if (HasAuthority())
    {
        Multicast_PlayFireAnimation();
    }

    
    FVector StartLocation = MuzzleLocation ? MuzzleLocation->GetComponentLocation() : GetActorLocation();

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            FireSound,
            StartLocation
        );
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
  
  
    // 서버에서만 데미지 처리 및 라인 트레이스 실행
    if (HasAuthority())
    {
        PerformLineTrace(StartLocation, FinalFireDirection);
        
        // 이펙트를 모든 클라이언트에서 재생
        Multicast_PlayFireEffects(StartLocation, FinalFireDirection);
    }

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
    // Server_Reload RPC를 통해 서버에서 호출되므로, 서버 권한 체크는 Server_Reload에서 수행
    // WeaponReload는 Server_Reload_Implementation에서 호출되므로 서버에서 실행됨이 보장됨
    // 클라이언트에서 직접 호출하는 경우를 방지하기 위한 체크는 유지하되, 
    // Server_Reload를 통한 정상적인 호출은 문제없이 작동해야 함
    
    
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

    bIsReloading = true; 
    bCanFire = false;    

    // 서버에서만 멀티캐스트 호출 (모든 클라이언트에서 재장전 애니메이션 재생)
    if (HasAuthority())
    {
        Multicast_PlayReloadAnimation();
    }

    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            ReloadSound,
            GetActorLocation()
        );
    }

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
    // 서버에서만 실행되어야 함 (인벤토리 수정 및 탄약 변경은 서버 권한 필요)
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: FinishReload called on client. This should only run on server."), *GetName());
        return;
    }

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

void AWeapon::Multicast_PlayFireAnimation_Implementation()
{
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
}

void AWeapon::Multicast_PlayReloadAnimation_Implementation()
{
    // 모든 클라이언트에서 재장전 애니메이션 재생
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    
    UE_LOG(LogTemp, Log, TEXT("%s: Multicast_PlayReloadAnimation called on %s, OwnerPawn: %s, ReloadAnimMontage: %s"), 
        *GetName(), 
        HasAuthority() ? TEXT("Server") : TEXT("Client"),
        OwnerPawn ? *OwnerPawn->GetName() : TEXT("NULL"),
        ReloadAnimMontage ? TEXT("Valid") : TEXT("NULL"));
    
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Multicast_PlayReloadAnimation - OwnerPawn is null!"), *GetName());
        return;
    }
    
    if (!ReloadAnimMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Multicast_PlayReloadAnimation - ReloadAnimMontage is not set! Please set it in Blueprint."), *GetName());
        // ReloadAnimMontage가 없어도 블루프린트 이벤트는 호출
        OnReloadEvent();
        return;
    }
    
    USkeletalMeshComponent* CharacterMesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Multicast_PlayReloadAnimation - CharacterMesh not found!"), *GetName());
        OnReloadEvent();
        return;
    }

    UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Multicast_PlayReloadAnimation - AnimInstance not found!"), *GetName());
        OnReloadEvent();
        return;
    }

    // 애니메이션 몽타주 재생
    float MontageLength = AnimInstance->Montage_Play(ReloadAnimMontage, 1.0f);
    if (MontageLength > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("%s: Successfully playing reload animation montage (Length: %f) on %s"), 
            *GetName(), 
            MontageLength,
            HasAuthority() ? TEXT("Server") : TEXT("Client"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Failed to play reload animation montage!"), *GetName());
    }
    
    // 블루프린트 이벤트도 호출 (추가 효과용)
    OnReloadEvent();
}

void AWeapon::Multicast_PlayFireEffects_Implementation(FVector StartLocation, FVector Direction)
{
    // 머즐 플래시 이펙트 재생
    if (MuzzleLocation && MuzzleFlashFX)
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

    // 트레이서 생성 (시각적 효과만, 데미지는 서버에서 처리)
    if (TracerActorClass)
    {
        // 트레이서의 목표 위치 계산 (최대 사거리까지)
        FVector EndLocation = StartLocation + (Direction * MaxRange);
        
        // 간단한 라인 트레이스로 목표 위치 찾기 (시각적 효과용)
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            Params.AddIgnoredActor(OwnerPawn);
        }

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            StartLocation,
            EndLocation,
            ECollisionChannel::ECC_Visibility,
            Params
        );

        FVector TargetLocation = bHit ? HitResult.Location : EndLocation;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            SpawnParams.Instigator = OwnerPawn;
        }

        // 트레이서 액터 생성
        AATracer* TracerActor = GetWorld()->SpawnActor<AATracer>(
            TracerActorClass,
            StartLocation,
            Direction.Rotation(),
            SpawnParams
        );

        if (TracerActor)
        {
            const float BulletSpeed = 20000.0f;
            TracerActor->StartMoving(TargetLocation, BulletSpeed);
        }
    }
}
