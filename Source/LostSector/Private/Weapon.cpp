// AWeapon.cpp
#include "Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h" 
#include "EngineUtils.h"
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

void AWeapon::Fire(FVector Direction)
{
    if (!bCanFire || CurrentAmmo <= 0)
    {
        // [추가] 발사 불가 시 로그 출력
        UE_LOG(LogTemp, Warning, TEXT("Fire Blocked: bCanFire=%s, Ammo=%d"),
            bCanFire ? TEXT("True") : TEXT("False"), CurrentAmmo);
        return;
    }

    // 1. [핵심] 첫 발 여부 확인
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    // 마지막 발사 후 SpreadResetDuration보다 긴 시간이 지났다면 첫 발로 간주 (분산 미적용)
    const bool bIsFirstShot = (CurrentTime - LastFireTime > SpreadResetDuration);

    // 2. 마지막 발사 시간 업데이트
    LastFireTime = CurrentTime;

    // [추가] 발사 시작 시 로그 출력
    UE_LOG(LogTemp, Log, TEXT("Fire Start! Ammo Left: %d, First Shot: %s"), CurrentAmmo - 1, bIsFirstShot ? TEXT("True") : TEXT("False"));

    // 발사 로직 실행
    bCanFire = false;
    CurrentAmmo--;

    // ----------------------------------------------------
    // [핵심] 총알 분산 (Aim Spread) 로직 적용
    // ----------------------------------------------------
    FVector StartLocation = MuzzleLocation->GetComponentLocation();

    // 최종 발사 방향을 Direction으로 초기화합니다.
    FVector FinalFireDirection = Direction;

    // 첫 발이 아니거나 (연사 중이거나), SpreadAngle이 0보다 커야 분산을 적용합니다.
    if (!bIsFirstShot && SpreadAngle > 0.0f)
    {
        // 1. 현재 방향(Direction)을 회전값으로 변환
        const FRotator CurrentRotator = Direction.Rotation();

        // 2. SpreadAngle 범위 내에서 무작위 각도(Yaw, Pitch) 생성
        float RandomYaw = FMath::FRandRange(-SpreadAngle, SpreadAngle);
        float RandomPitch = FMath::FRandRange(-SpreadAngle, SpreadAngle);

        // 3. 현재 회전값에 무작위 분산 각도를 더함
        const FRotator SpreadRotator = CurrentRotator + FRotator(RandomPitch, RandomYaw, 0.0f);

        // 4. 새로운 회전값으로 변환된 최종 발사 방향을 얻음
        FinalFireDirection = UKismetMathLibrary::GetForwardVector(SpreadRotator);
    }
    // [첫 발인 경우] FinalFireDirection은 Direction 그대로 유지되어 정확하게 발사됩니다.

    PerformLineTrace(StartLocation, FinalFireDirection); // **수정된 방향 전달**

    OnFireEvent();

    // 연사 속도 타이머 설정
    GetWorld()->GetTimerManager().SetTimer(
        FireRateTimerHandle,
        this,
        &AWeapon::ResetFire,
        FireRate,
        false
    );
}