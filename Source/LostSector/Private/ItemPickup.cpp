#include "ItemPickup.h"
#include "ItemDataBase.h"                       // ✅ DataAsset 필드 접근용
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

    SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMeshComp->SetupAttachment(RootComponent);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SkeletalMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    SkeletalMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    SkeletalMeshComp->SetVisibility(false, true);
}

void AItemPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AItemPickup, Stack);
}

// 레벨에 놓거나 Details에서 값 바꿀 때 바로 반영
void AItemPickup::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyVisualFromData();
}

// 클라에서 Stack 복제되면 외형 갱신
void AItemPickup::OnRep_Stack()
{
    ApplyVisualFromData();
}

void AItemPickup::ApplyVisualFromData()
{
    // 기본 숨김
    StaticMeshComp->SetVisibility(false, true);
    SkeletalMeshComp->SetVisibility(false, true);

    if (!Stack.Item) return;

    // DataAsset에 넣어둔 메쉬/보정값을 적용
    if (Stack.Item->WorldStaticMesh)
    {
        StaticMeshComp->SetStaticMesh(Stack.Item->WorldStaticMesh.Get()); // TSoftObjectPtr이면 .Get() 또는 LoadSynchronous()
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
    // 강제로 (0,0,0) 되지 않도록 방지 — 위치 리셋 없음
}


void AItemPickup::Interact(ACharacter* ByWho)
{
    UE_LOG(LogTemp, Warning, TEXT(" Pickup Interact (Authority=%d)"), GetLocalRole() == ROLE_Authority);

    if (!ByWho || GetLocalRole() != ROLE_Authority) return;
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