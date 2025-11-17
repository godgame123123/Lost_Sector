// AWeapon.cpp
#include "Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

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
    // 초기 탄약 설정
    CurrentAmmo = MaxAmmo;
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

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility, // 필요에 따라 변경
        Params
    );

    FColor LineColor = bHit ? FColor::Red : FColor::Green;

    // DrawDebugLine 함수는 Kismet/KismetMathLibrary.h 에 정의되어 있습니다.
    // 현재 코드에는 #include "Kismet/KismetMathLibrary.h" 가 포함되어 있으므로 바로 사용 가능합니다.
    DrawDebugLine(
        GetWorld(),
        Start,
        bHit ? HitResult.Location : End, // 히트했으면 히트 지점까지, 아니면 최대 사거리까지
        LineColor,
        false,      // bPersistentLines (영구적이지 않음)
        5.0f,       // LifeTime (5초간 표시)
        0,          // DepthPriority
        3.0f        // Thickness (선의 두께)
    );

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

void AWeapon::Fire()
{
    if (!bCanFire || CurrentAmmo <= 0)
    {
        // [추가] 발사 불가 시 로그 출력
        UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: bCanFire=%s, Ammo=%d"),
            bCanFire ? TEXT("True") : TEXT("False"), CurrentAmmo);
        return;
    }

    // [추가] 발사 시작 시 로그 출력
    UE_LOG(LogTemp, Log, TEXT("Fire Start! Ammo Left: %d"), CurrentAmmo - 1);

    // 발사 로직 실행
    bCanFire = false;
    CurrentAmmo--;

    // ----------------------------------------------------
    // [핵심] 쿼터뷰 발사 방향 결정 로직
    // ----------------------------------------------------
    FVector StartLocation = MuzzleLocation->GetComponentLocation();

    // 무기 액터의 정면 방향을 발사 방향으로 사용
    FVector FireDirection = WeaponMesh->GetForwardVector();

    // 만약 플레이어의 마우스 방향으로 쏘게 하려면, 
    // 캐릭터 컨트롤러에서 마우스 커서 위치를 가져와 Direction을 재계산해야 합니다.
    // (현재는 기본: 무기가 바라보는 방향)

    PerformLineTrace(StartLocation, FireDirection);

    // 연사 속도 타이머 설정
    GetWorld()->GetTimerManager().SetTimer(
        FireRateTimerHandle,
        this,
        &AWeapon::ResetFire,
        FireRate,
        false
    );
}