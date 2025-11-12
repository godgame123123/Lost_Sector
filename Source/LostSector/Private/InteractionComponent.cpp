#include "InteractionComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Interactable.h"

void UInteractionComponent::BeginPlay() { Super::BeginPlay(); }

void UInteractionComponent::Use()
{
    if (ACharacter* C = Cast<ACharacter>(GetOwner()))
    {
        FVector L; FRotator R; C->GetActorEyesViewPoint(L, R);
        Server_Use(L, R); // 서버가 직접 트레이스
    }
}

void UInteractionComponent::Server_Use_Implementation(const FVector_NetQuantize& EyeLoc, const FRotator& EyeRot)
{
    ACharacter* C = Cast<ACharacter>(GetOwner()); if (!C) return;
    const FVector End = EyeLoc + EyeRot.Vector() * Range;
    FHitResult Hit; FCollisionQueryParams Params(SCENE_QUERY_STAT(UseTrace), false, C);
    GetWorld()->LineTraceSingleByChannel(Hit, EyeLoc, End, ECC_Visibility, Params);

    AActor* Target = Hit.GetActor(); if (!Target) return;
    if (Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
        if (IInteractable* I = Cast<IInteractable>(Target)) I->Interact(C);
}
