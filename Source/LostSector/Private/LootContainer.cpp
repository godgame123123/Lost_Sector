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

void ALootContainer::InitializeLoot(const TArray<FItemStack>& InventoryToStore)
{
    if (!Inventory)
    {
        UE_LOG(LogTemp, Error, TEXT("ALootContainer::InitializeLoot failed: Inventory component is null."));
        return;
    }
    // 기존 인벤토리를 비우고 (필요하다면)
    Inventory->Slots.Empty();

    // 전달받은 아이템 목록으로 인벤토리를 채웁니다.
    // TArray는 Deep Copy를 지원하므로 바로 복사가 가능합니다.
    Inventory->Slots = InventoryToStore;

    // 인벤토리 컴포넌트에게 슬롯이 변경되었음을 알립니다 (복제 및 UI 업데이트를 위해 필요할 수 있음)
    Inventory->BroadcastUpdated(); // InventoryComponent에 이런 함수가 있다고 가정

    // 만약 InventoryComponent가 별도의 초기화 로직을 요구한다면 호출합니다.
    // Inventory->RefreshCapacity(); // 예시: 무게/슬롯 업데이트

    UE_LOG(LogTemp, Log, TEXT("LootContainer initialized with %d item stacks."), InventoryToStore.Num());
}