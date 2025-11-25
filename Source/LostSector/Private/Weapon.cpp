// AWeapon.cpp
#include "Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h" 
#include "EngineUtils.h"
#include "NiagaraFunctionLibrary.h" // 나이아가라 함수 라이브러리
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h" // 스켈레탈 메시 컴포넌트 접근을 위해
#include "ATracer.h"
#include "Animation/AnimInstance.h"
#include "InventoryComponent.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    // 루트 컴포넌트 생성
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // 스태틱 메쉬 컴포넌트 생성 및 루트에 부착
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(RootComponent);

    // 총구 위치 컴포넌트 생성 (발사 시작 지점)
    MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
    MuzzleLocation->SetupAttachment(WeaponMesh.Get());

    NoiseRange = 5000.0f;
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    // BeginPlay 시점에 메쉬 보정 값을 적용합니다.
    if (WeaponMesh)
    {
        // 1. 상대 위치 (Location) 및 회전 (Rotation) 적용
        WeaponMesh->SetRelativeLocation(MeshOffsetLocation);
        WeaponMesh->SetRelativeRotation(MeshOffsetRotation);

        // 2. 스케일도 보정해야 한다면 (총 크기가 다를 경우)
        // WeaponMesh->SetRelativeScale3D(FVector(1.0f)); // 필요한 스케일 값 적용
    }
}

void AWeapon::ResetFire()
{
    bCanFire = true;
}

void AWeapon::PerformLineTrace(FVector Start, FVector Direction)
{
    // [히트스캔 로직]
    FVector End = Start + (Direction * MaxRange);
    FHitResult HitResult;

    // Trace Channel을 Visibility 또는 Custom Trace Channel로 설정
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); // 무기 액터는 트레이스에서 제외

    AController* InstigatorController = GetInstigatorController();
    AActor* InstigatorPawn = GetInstigator();

    if (InstigatorPawn)
    {
        Params.AddIgnoredActor(InstigatorPawn); // 발사자(Instigator Pawn)도 무시 목록에 추가

        // 발사자가 AI Controller의 제어를 받고 있다면 (즉, AI가 공격 중이라면)
        if (InstigatorController && InstigatorController->IsA<AAIController>())
        {
            UWorld* World = GetWorld();
            if (World)
            {
                // 월드 내의 모든 Pawn (캐릭터)을 순회합니다.
                for (TActorIterator<APawn> It(World); It; ++It)
                {
                    APawn* Pawn = *It;

                    // 1. Pawn이 유효하고 2. 플레이어가 제어하지 않는 경우 (즉, 다른 AI/NPC인 경우)
                    // (추가적으로 팀 체크 로직을 넣을 수 있지만, 여기서는 간단히 AI/NPC로 구분)
                    if (Pawn && !Pawn->IsPlayerControlled())
                    {
                        // 해당 AI/NPC 캐릭터를 라인 트레이스 무시 목록에 추가합니다.
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
        ECollisionChannel::ECC_Visibility, // 필요에 따라 변경
        Params
    );

    FVector TargetLocation = bHit ? HitResult.Location : End;

    if (TracerActorClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        // 트레이서 액터를 총구 위치(Start)에 생성
        AATracer* TracerActor = GetWorld()->SpawnActor<AATracer>(
            TracerActorClass,
            Start,
            Direction.Rotation(),
            SpawnParams
        );

        if (TracerActor)
        {
            // 2. [이동 지시] TargetLocation과 속도를 전달합니다.
            //     이 함수 호출을 누락하면 AATracer의 TargetLocation 변수는 기본값 (FVector::ZeroVector)으로 남아있게 됩니다.

            const float BulletSpeed = 20000.0f; // 매우 빠른 속도로 설정 (단위: cm/s)

            // AATracer::StartMoving 함수 호출
            TracerActor->StartMoving(TargetLocation, BulletSpeed);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn AATracer actor!"));
        }
    }
    

    // 2. 히트 임팩트 이펙트 생성 (맞았을 경우에만)
    if (bHit && HitImpactFX) // AWeapon.h에 선언된 UNiagaraSystem* HitImpactFX 변수 사용
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            HitImpactFX,
            HitResult.Location,
            HitResult.ImpactNormal.Rotation(), // 맞은 면의 법선 방향으로 회전
            FVector(1.0f),
            true,
            true
        );
    }

    //FColor LineColor = bHit ? FColor::Red : FColor::Green;

    // DrawDebugLine 함수는 Kismet/KismetMathLibrary.h 에 정의되어 있습니다.
    // 현재 코드에는 #include "Kismet/KismetMathLibrary.h" 가 포함되어 있으므로 바로 사용 가능합니다.
    //DrawDebugLine(
    //    GetWorld(),
    //    Start,
    //    bHit ? HitResult.Location : End, // 히트했으면 히트 지점까지, 아니면 최대 사거리까지
    //    LineColor,
    //    false,      // bPersistentLines (영구적이지 않음)
    //    5.0f,       // LifeTime (5초간 표시)
    //    0,          // DepthPriority
    //    3.0f        // Thickness (선의 두께)
    //);

    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();
        if (HitActor)
        {
            // 데미지 적용
            UGameplayStatics::ApplyDamage(
                HitActor,
                Damage,
                GetInstigatorController(), // 발사한 캐릭터의 컨트롤러를 데미지 유발자로 전달
                this,
                nullptr // DamageTypeClass
            );

            // TODO: 피격 이펙트 및 사운드 생성 (HitResult.Location, HitResult.ImpactNormal 사용)
        }
    }

    // TODO: 총구 화염 (Muzzle Flash) 및 탄피 배출 이펙트 생성
}

void AWeapon::Fire(FVector Direction)
{

    if (!bCanFire)
    {
        UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: bCanFire=false"));
        return;
    }

    // CurrentAmmo가 0이면 인벤토리에서 총알 확인
    if (CurrentAmmo <= 0)
    {
        // 인벤토리에서 총알 수량 확인
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
                    // 인벤토리에 총알이 있으면 자동으로 재장전 시도
                    UE_LOG(LogTemp, Log, TEXT("Auto-reload: Found %d ammo in inventory"), AmmoCount);
                    WeaponReload();
                    return; // 재장전 중이므로 발사 불가
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

    // 기존 발사 로직 계속
    if (false) // 이전 체크는 위에서 처리했으므로 여기서는 항상 false
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
    // 1. 이미 최대 탄약이라면 재장전 불필요
    if (CurrentAmmo >= MaxAmmo)
    {
        UE_LOG(LogTemp, Log, TEXT("%s: Ammo is already full (%d/%d)."), *GetName(), CurrentAmmo, MaxAmmo);
        return;
    }

    // 2. 소유자(Pawn)와 인벤토리 컴포넌트 찾기
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

    // 3. 필요한 탄약 수 계산 및 인벤토리에서 사용 가능한 총알 수 확인
    int32 AmmoNeeded = MaxAmmo - CurrentAmmo;
    int32 AvailableAmmo = InventoryComp->GetItemCountByItemData(RequiredAmmoItemData);

    if (AvailableAmmo <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Reload Failed: No available ammo in inventory."));
        return;
    }

    // 4. 실제로 인벤토리에서 가져올/제거할 탄약 수 결정
    int32 AmmoToTake = FMath::Min(AmmoNeeded, AvailableAmmo);

    // 5. 인벤토리에서 탄약 제거 요청 (인벤토리 함수는 서버 권한으로 실행됨)
    // UInventoryComponent에 Add/Remove 로직이 서버 권한으로 분리되어 있다면,
    // 클라이언트에서 이 함수를 호출할 경우 내부적으로 RPC가 발생합니다.
    int32 RemovedCount = InventoryComp->RemoveItemByItemData(RequiredAmmoItemData, AmmoToTake);

    // 6. 탄창 업데이트
    if (RemovedCount > 0)
    {
        CurrentAmmo += RemovedCount;
        UE_LOG(LogTemp, Log, TEXT("%s: Reloaded %d rounds. New Ammo: %d/%d. Inventory Count: %d"),
            *GetName(), RemovedCount, CurrentAmmo, MaxAmmo, AvailableAmmo - RemovedCount);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Reload Error: Inventory refused to remove ammo unexpectedly."));
    }
}
