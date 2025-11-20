// Fill out your copyright notice in the Description page of Project Settings.


#include "ATracer.h"
#include "NiagaraComponent.h" // UNiagaraComponent 구현을 위해 포함
#include "Kismet/KismetMathLibrary.h" // FVector::Distance 사용을 위해

// Sets default values
AATracer::AATracer()
{
    PrimaryActorTick.bCanEverTick = true;

    // 루트 컴포넌트 설정
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

    // 나이아가라 컴포넌트 생성 및 루트에 부착
    NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
    NiagaraComp->SetupAttachment(RootComponent);

    // 이펙트 에셋은 블루프린트에서 설정할 예정이므로 여기서는 비활성화 상태로 시작
    NiagaraComp->SetAutoActivate(false);
}

// Called when the game starts or when spawned
void AATracer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AATracer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 이동이 시작되지 않았거나, 이미 끝났다면 Tick 로직을 실행하지 않음
    if (TotalFlightTime <= 0.0f)
    {
        return;
    }

    // 1. 경과 시간 업데이트
    ElapsedTime += DeltaTime;

    // 2. 보간 값 (Alpha) 계산
    float Alpha = ElapsedTime / TotalFlightTime;

    // 3. 이동 처리
    if (Alpha < 1.0f)
    {
        // Lerp 함수를 사용하여 현재 위치를 시작점과 목표점 사이로 보간
        FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, Alpha);
        SetActorLocation(NewLocation);

        // 파티클이 날아가는 방향으로 회전 (선택 사항)
        FRotator NewRotation = (TargetLocation - NewLocation).Rotation();
        SetActorRotation(NewRotation);
    }
    else
    {
        // 4. 목표 지점 도달 시 액터 파괴
        SetActorLocation(TargetLocation); // 마지막 위치 보정

        // 나이아가라 시스템이 루프하지 않도록 AutoDestroy를 명시적으로 호출
        NiagaraComp->Deactivate();

        // 일정 딜레이 후 파괴 (파티클 잔상이 사라질 시간을 줌)
        SetLifeSpan(0.1f); // 0.1초 후 액터 자체를 파괴

        // Tick 중지
        PrimaryActorTick.bCanEverTick = false;
    }
}

void AATracer::StartMoving(const FVector Target, float Speed)
{
    StartLocation = GetActorLocation();
    TargetLocation = Target;

    // 2. 이동 시간 계산 (거리 / 속도)
    const float Distance = FVector::Dist(StartLocation, TargetLocation);
    // 무한 루프 방지
    if (FMath::IsNearlyZero(Distance))
    {
        TotalFlightTime = 0.001f; // 거리가 0이면 최소 시간 설정
    }
    else
    {
        TotalFlightTime = Distance / Speed;
    }

    // 3. 나이아가라 활성화 (이동 시작)
    if (NiagaraComp)
    {
        NiagaraComp->Activate(true);
    }
}

