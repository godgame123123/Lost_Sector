#include "ItemPickup.h"
#include "ItemDataBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "InventoryComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AItemPickup::AItemPickup()
{
    bReplicates = true;

    StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    SetRootComponent(StaticMeshComp);

    StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    StaticMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    StaticMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    StaticMeshComp->SetGenerateOverlapEvents(true);
    StaticMeshComp->SetCollisionObjectType(ECC_WorldDynamic);
    StaticMeshComp->bTraceComplexOnMove = true;
    StaticMeshComp->bReturnMaterialOnMove = true;

    SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMeshComp->SetupAttachment(RootComponent);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SkeletalMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    SkeletalMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    SkeletalMeshComp->SetGenerateOverlapEvents(true);
    SkeletalMeshComp->bTraceComplexOnMove = true;
    SkeletalMeshComp->bReturnMaterialOnMove = true;
}

void AItemPickup::BeginPlay()
{
    Super::BeginPlay();
    // ❌ 중복 호출 제거. OnRep_Stack이 자동으로 처리함.
    // ApplyVisualFromData();
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

    if (Stack.Item->WorldStaticMesh)
    {
        StaticMeshComp->SetStaticMesh(Stack.Item->WorldStaticMesh.Get());
        StaticMeshComp->SetRelativeRotation(Stack.Item->WorldMeshRotation);
        StaticMeshComp->SetRelativeLocation(Stack.Item->WorldMeshOffset);
        StaticMeshComp->SetRelativeScale3D(FVector(Stack.Item->WorldMeshScale));
        StaticMeshComp->SetVisibility(true, true);
    }
    else if (Stack.Item->WorldSkeletalMesh)
    {
        SkeletalMeshComp->SetSkeletalMesh(Stack.Item->WorldSkeletalMesh.Get());
        SkeletalMeshComp->SetRelativeRotation(Stack.Item->WorldMeshRotation);
        SkeletalMeshComp->SetRelativeLocation(Stack.Item->WorldMeshOffset);
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
