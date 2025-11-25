#include "InteractionComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Interactable.h"

UInteractionComponent::UInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UInteractionComponent::Use()
{
    if (ACharacter* C = Cast<ACharacter>(GetOwner()))
    {
        FVector EyeLoc;
        FRotator EyeRot;

        C->GetActorEyesViewPoint(EyeLoc, EyeRot);

        Server_Use(EyeLoc, EyeRot); // 서버 호출
    }
}

void UInteractionComponent::Server_Use_Implementation(
    const FVector_NetQuantize& EyeLoc,
    const FRotator& EyeRot)
{
    ACharacter* C = Cast<ACharacter>(GetOwner());
    if (!C) return;

    FVector End = EyeLoc + EyeRot.Vector() * Range;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(C);
    Params.bTraceComplex = true;

    //  핵심: LineTrace → SphereTrace 로 변경해서 부드러운 상호작용 구현
    bool bHit = GetWorld()->SweepSingleByChannel(
        Hit,
        EyeLoc,
        End,
        FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(SphereRadius),
        Params
    );

    //  디버그 표시 (원하면 삭제)
    // DrawDebugSphere(GetWorld(), Hit.Location, SphereRadius, 12, FColor::Green, false, 1.5f);
    // DrawDebugLine(GetWorld(), EyeLoc, End, FColor::Yellow, false, 1.5f);

    if (!bHit) return;
    AActor* Target = Hit.GetActor();
    if (!Target) return;

    UE_LOG(LogTemp, Warning, TEXT("Interaction Hit: %s"), *Target->GetName());

    // 인터페이스 실행
    if (Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        if (IInteractable* I = Cast<IInteractable>(Target))
        {
            I->Interact(C);
        }
    }
}
