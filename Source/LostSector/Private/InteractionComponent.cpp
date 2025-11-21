#include "InteractionComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
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
    UE_LOG(LogTemp, Warning, TEXT("Use() called"));  //  E키가 여기까지 오는지

    if (ACharacter* C = Cast<ACharacter>(GetOwner()))
    {
        FVector L;
        FRotator R;
        C->GetActorEyesViewPoint(L, R);
        Server_Use(L, R);
    }
}

void UInteractionComponent::Server_Use_Implementation(const FVector_NetQuantize& EyeLoc, const FRotator& EyeRot)
{
    UE_LOG(LogTemp, Warning, TEXT("Server_Use_Implementation"));  // 서버 RPC 호출 여부

    ACharacter* C = Cast<ACharacter>(GetOwner());
    if (!C) return;

    const FVector End = EyeLoc + EyeRot.Vector() * Range;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(UseTrace), false, C);
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, EyeLoc, End, ECC_Visibility, Params);

    UE_LOG(LogTemp, Warning, TEXT("Trace hit: %s"),
        (bHit && Hit.GetActor()) ? *Hit.GetActor()->GetName() : TEXT("None"));  

    AActor* Target = Hit.GetActor();
    if (!Target) return;

    if (Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Target implements Interactable"));

        if (IInteractable* I = Cast<IInteractable>(Target))
        {
            UE_LOG(LogTemp, Warning, TEXT("Calling Interact()"));
            I->Interact(C);
        }
    }
}