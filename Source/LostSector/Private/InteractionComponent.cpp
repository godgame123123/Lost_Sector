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
        // 서버에게 캐릭터 위치와 방향 전달
        FVector Start = C->GetActorLocation();
        FVector Forward = C->GetActorForwardVector();
        UE_LOG(LogTemp, Warning, TEXT("Use Pressed"));

        Server_Use(Start, Forward.Rotation());
    }
}

void UInteractionComponent::Server_Use_Implementation(
    const FVector_NetQuantize& StartLoc,
    const FRotator& FacingRot)
{
    ACharacter* C = Cast<ACharacter>(GetOwner());
    if (!C) return;

    // 캐릭터 기준 방향
    FVector Forward = FacingRot.Vector();
    FVector End = StartLoc + Forward * Range;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(C);
    Params.bTraceComplex = true;

    bool bHit = GetWorld()->SweepSingleByChannel(
        Hit,
        StartLoc,
        End,
        FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(SphereRadius),
        Params
    );
    if (!bHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("Trace: No Hit"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Trace Hit: %s"), *Hit.GetActor()->GetName());
    }



    // 디버그용
    //DrawDebugSphere(GetWorld(), Hit.Location, SphereRadius, 12, FColor::Cyan, false, 1.5f);
    //DrawDebugLine(GetWorld(), StartLoc, End, FColor::Blue, false, 1.5f);

    if (!bHit) return;

    AActor* Target = Hit.GetActor();
    if (!Target) return;

    UE_LOG(LogTemp, Warning, TEXT("Interaction Hit: %s"), *Target->GetName());

    if (Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        IInteractable::Execute_Interact(Target, C);
    }
}
