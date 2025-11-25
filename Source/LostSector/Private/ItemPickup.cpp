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

    // -----------------------------
    // Static Mesh Component (Root)
    // -----------------------------
    StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    SetRootComponent(StaticMeshComp);

    // 🔥 핵심 개선: 전체 Mesh 충돌 감지되도록 설정
    StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    StaticMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    StaticMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    StaticMeshComp->SetGenerateOverlapEvents(true);
    StaticMeshComp->SetCollisionObjectType(ECC_WorldDynamic);

    // 🔥 이거 켜야 SphereTrace / 라인트레이스가 "메쉬 전체"에 닿음
    StaticMeshComp->bTraceComplexOnMove = true;
    StaticMeshComp->bReturnMaterialOnMove = true;


    // -----------------------------
    // Skeletal Mesh Component
    // -----------------------------
    SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMeshComp->SetupAttachment(RootComponent);

    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SkeletalMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    SkeletalMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    SkeletalMeshComp->SetGenerateOverlapEvents(true);
    SkeletalMeshComp->bTraceComplexOnMove = true;
    SkeletalMeshComp->bReturnMaterialOnMove = true;

    SkeletalMeshComp->SetVisibility(false, true);
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

    if (!Stack.Item) return;

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

void AItemPickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    // 위치 강제 리셋 방지 (0,0,0로 튀는 현상 대응)
}

void AItemPickup::Interact(ACharacter* ByWho)
{
    UE_LOG(LogTemp, Warning, TEXT("Pickup Interact (Authority=%d)"), GetLocalRole() == ROLE_Authority);

    if (!ByWho || GetLocalRole() != ROLE_Authority)
        return;

    if (!Stack.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Stack invalid"));
        return;
    }

    if (FVector::Dist(ByWho->GetActorLocation(), GetActorLocation()) > MaxUseDistance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Too far"));
        return;
    }

    if (UInventoryComponent* Inv = ByWho->FindComponentByClass<UInventoryComponent>())
    {
        int32 Added = 0;
        Inv->TryAddStack(Stack, Added);
        UE_LOG(LogTemp, Warning, TEXT("TryAddStack Added = %d"), Added);

        if (Added > 0)
        {
            Stack.Count -= Added;
            if (Stack.Count <= 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Destroy pickup"));
                Destroy();
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No InventoryComponent on character"));
    }
}
