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
    StaticMeshComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMeshComp->SetupAttachment(RootComponent);
    SkeletalMeshComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
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

void AItemPickup::Interact(ACharacter* ByWho)
{
    if (!ByWho || GetLocalRole() != ROLE_Authority || !Stack.IsValid()) return;
    if (FVector::Dist(ByWho->GetActorLocation(), GetActorLocation()) > MaxUseDistance) return;

    if (UInventoryComponent* Inv = ByWho->FindComponentByClass<UInventoryComponent>())
    {
        int32 Added = 0;
        Inv->TryAddStack(Stack, Added);
        if (Added > 0)
        {
            Stack.Count -= Added;
            if (Stack.Count <= 0) Destroy();
            else OnRep_Stack(); // 서버에서도 외형 최신화(부분 남았을 때)
        }
    }
}
