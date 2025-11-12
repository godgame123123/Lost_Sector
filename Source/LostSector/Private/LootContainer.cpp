#include "LootContainer.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"   // ✅ 여기 포함

ALootContainer::ALootContainer()
{
    bReplicates = true;                               // ✅ 액터 복제 켜기
    Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void ALootContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);     // ✅ 인자명 일치
    DOREPLIFETIME(ALootContainer, bLocked);
}

bool ALootContainer::TryLock(AController* By)
{
    if (bLocked) return LockedBy.IsValid() && LockedBy.Get() == By;
    bLocked = true; LockedBy = By; return true;
}
void ALootContainer::Unlock(AController* By)
{
    if (LockedBy.IsValid() && LockedBy.Get() == By) { bLocked = false; LockedBy.Reset(); }
}

void ALootContainer::Interact(ACharacter* ByWho)
{
    if (!ByWho || GetLocalRole() != ROLE_Authority) return;
    if (FVector::Dist(ByWho->GetActorLocation(), GetActorLocation()) > MaxUseDistance) return;

    AController* Ctrl = ByWho->GetController(); if (!TryLock(Ctrl)) return;
    if (UInventoryComponent* PlayerInv = ByWho->FindComponentByClass<UInventoryComponent>())
    {
        int32 Moved = 0; PlayerInv->TransferAllFrom(Inventory, Moved);
    }
    Unlock(Ctrl);
}
