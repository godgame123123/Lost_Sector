#include "ItemPickup.h"
#include "ItemDataBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "InventoryComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AItemPickup::AItemPickup()
{
    bReplicates = true;

    /** 🔹 Root */
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    /** 🔹 Static Mesh */
    StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMeshComp->SetupAttachment(SceneRoot);

    StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    StaticMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    StaticMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    StaticMeshComp->SetGenerateOverlapEvents(true);
    StaticMeshComp->SetCollisionObjectType(ECC_WorldDynamic);

    /** 🔹 Skeletal Mesh */
    SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMeshComp->SetupAttachment(SceneRoot);

    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SkeletalMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    SkeletalMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    SkeletalMeshComp->SetGenerateOverlapEvents(true);
}

void AItemPickup::BeginPlay()
{
    Super::BeginPlay();
}

void AItemPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AItemPickup, Stack);
}

void AItemPickup::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyVisualFromData();
}

#if WITH_EDITOR
void AItemPickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property &&
        PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AItemPickup, Stack))
    {
        ApplyVisualFromData();
    }
}
#endif

void AItemPickup::OnRep_Stack()
{
    ApplyVisualFromData();
}

void AItemPickup::ApplyVisualFromData()
{
    StaticMeshComp->SetVisibility(false, true);
    SkeletalMeshComp->SetVisibility(false, true);

    if (!Stack.Item)
        return;

    /** Static Mesh */
    if (Stack.Item->WorldStaticMesh)
    {
        StaticMeshComp->SetStaticMesh(Stack.Item->WorldStaticMesh.Get());
        StaticMeshComp->SetRelativeLocation(Stack.Item->WorldMeshOffset);
        StaticMeshComp->SetRelativeRotation(Stack.Item->WorldMeshRotation);
        StaticMeshComp->SetRelativeScale3D(FVector(Stack.Item->WorldMeshScale));
        StaticMeshComp->SetVisibility(true, true);
    }
    /** Skeletal Mesh */
    else if (Stack.Item->WorldSkeletalMesh)
    {
        SkeletalMeshComp->SetSkeletalMesh(Stack.Item->WorldSkeletalMesh.Get());
        SkeletalMeshComp->SetRelativeLocation(Stack.Item->WorldMeshOffset);
        SkeletalMeshComp->SetRelativeRotation(Stack.Item->WorldMeshRotation);
        SkeletalMeshComp->SetRelativeScale3D(FVector(Stack.Item->WorldMeshScale));
        SkeletalMeshComp->SetVisibility(true, true);
    }
}

void AItemPickup::Interact_Implementation(ACharacter* ByWho)
{
    if (!ByWho || GetLocalRole() != ROLE_Authority)
        return;

    if (!Stack.IsValid())
        return;

    if (FVector::Dist(ByWho->GetActorLocation(), GetActorLocation()) > MaxUseDistance)
        return;

    if (UInventoryComponent* Inv = ByWho->FindComponentByClass<UInventoryComponent>())
    {
        int32 Added = 0;
        Inv->TryAddStack(Stack, Added);

        if (Added > 0)
        {
            Stack.Count -= Added;

            if (Stack.Count <= 0)
            {
                Destroy();
            }
        }
    }
}

