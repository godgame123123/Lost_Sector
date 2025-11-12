#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "ItemPickup.h"
#include "InventorySaveManager.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetOwnerRole() == ROLE_Authority)
    {
        InitSlots();
    }
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UInventoryComponent, Slots);
}

void UInventoryComponent::InitSlots()
{
    Slots.Empty();
    for (int32 i = 0; i < SlotCount; i++)
    {
        Slots.Add(FItemStack());
    }
}

float UInventoryComponent::GetTotalWeight() const
{
    float Total = 0.f;
    for (const FItemStack& Stack : Slots)
    {
        if (Stack.Item)  // ← Item으로 수정
        {
            Total += Stack.Item->Weight * Stack.Count;
        }
    }
    return Total;
}

bool UInventoryComponent::CanAddWeight(float AddW) const
{
    return (GetTotalWeight() + AddW) <= WeightLimit;
}

bool UInventoryComponent::TryAddStack(const FItemStack& InStack, int32& OutAdded)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_TryAddStack(InStack);
        return false;
    }

    if (!InStack.Item || InStack.Count <= 0)  // ← Item으로 수정
    {
        OutAdded = 0;
        return false;
    }

    float SingleWeight = InStack.Item->Weight;  // ← Item으로 수정
    int32 Remaining = InStack.Count;
    OutAdded = 0;

    // 1단계: 기존 스택에 추가
    for (int32 i = 0; i < Slots.Num() && Remaining > 0; i++)
    {
        FItemStack& Slot = Slots[i];
        if (Slot.Item == InStack.Item && Slot.Count < InStack.Item->MaxStack)  // ← Item으로 수정
        {
            int32 CanAdd = FMath::Min(Remaining, InStack.Item->MaxStack - Slot.Count);  // ← Item으로 수정
            if (CanAddWeight(CanAdd * SingleWeight))
            {
                Slot.Count += CanAdd;
                Remaining -= CanAdd;
                OutAdded += CanAdd;
            }
        }
    }

    // 2단계: 빈 슬롯에 추가
    for (int32 i = 0; i < Slots.Num() && Remaining > 0; i++)
    {
        FItemStack& Slot = Slots[i];
        if (!Slot.Item)  // ← Item으로 수정
        {
            int32 CanAdd = FMath::Min(Remaining, InStack.Item->MaxStack);  // ← Item으로 수정
            if (CanAddWeight(CanAdd * SingleWeight))
            {
                Slot.Item = InStack.Item;  // ← Item으로 수정
                Slot.Count = CanAdd;
                Remaining -= CanAdd;
                OutAdded += CanAdd;
            }
        }
    }

    bool bSuccess = (OutAdded > 0);
    if (bSuccess)
    {
        BroadcastUpdated();
        ScheduleSave();
    }

    return bSuccess;
}

bool UInventoryComponent::TryMove(int32 FromIdx, int32 ToIdx)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_TryMove(FromIdx, ToIdx);
        return false;
    }

    if (!ValidIndex(FromIdx) || !ValidIndex(ToIdx) || FromIdx == ToIdx)
    {
        return false;
    }

    FItemStack Temp = Slots[FromIdx];
    Slots[FromIdx] = Slots[ToIdx];
    Slots[ToIdx] = Temp;

    BroadcastUpdated();
    ScheduleSave();
    return true;
}

bool UInventoryComponent::TrySplit(int32 FromIdx, int32 NumToSplit, int32 ToIdx)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_TrySplit(FromIdx, NumToSplit, ToIdx);
        return false;
    }

    if (!ValidIndex(FromIdx) || !ValidIndex(ToIdx) || FromIdx == ToIdx)
    {
        return false;
    }

    FItemStack& From = Slots[FromIdx];
    FItemStack& To = Slots[ToIdx];

    if (!From.Item || From.Count < NumToSplit || NumToSplit <= 0)  // ← Item으로 수정
    {
        return false;
    }

    if (To.Item)  // ← Item으로 수정
    {
        return false;
    }

    To.Item = From.Item;  // ← Item으로 수정
    To.Count = NumToSplit;
    From.Count -= NumToSplit;

    if (From.Count <= 0)
    {
        From.Item = nullptr;  // ← Item으로 수정
        From.Count = 0;
    }

    BroadcastUpdated();
    ScheduleSave();
    return true;
}

bool UInventoryComponent::RemoveAt(int32 Index, int32 Count)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        return false;
    }

    if (!ValidIndex(Index))
    {
        return false;
    }

    FItemStack& Slot = Slots[Index];
    if (!Slot.Item || Slot.Count < Count)  // ← Item으로 수정
    {
        return false;
    }

    Slot.Count -= Count;
    if (Slot.Count <= 0)
    {
        Slot.Item = nullptr;  // ← Item으로 수정
        Slot.Count = 0;
    }

    BroadcastUpdated();
    ScheduleSave();
    return true;
}

bool UInventoryComponent::TransferFrom(UInventoryComponent* From, int32 FromIdx, int32 Count, int32& OutMoved)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_TransferFrom(From, FromIdx, Count);
        return false;
    }

    if (!From || !From->ValidIndex(FromIdx))
    {
        return false;
    }

    FItemStack& FromSlot = From->Slots[FromIdx];
    if (!FromSlot.Item || FromSlot.Count < Count)  // ← Item으로 수정
    {
        return false;
    }

    FItemStack TransferStack;
    TransferStack.Item = FromSlot.Item;  // ← Item으로 수정
    TransferStack.Count = Count;

    int32 Added = 0;
    if (TryAddStack(TransferStack, Added))
    {
        From->RemoveAt(FromIdx, Added);
        OutMoved = Added;
        ScheduleSave();
        return true;
    }

    OutMoved = 0;
    return false;
}

bool UInventoryComponent::TransferAllFrom(UInventoryComponent* From, int32& OutTotalMoved)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_TransferAllFrom(From);
        return false;
    }

    if (!From)
    {
        return false;
    }

    OutTotalMoved = 0;
    for (int32 i = 0; i < From->Slots.Num(); i++)
    {
        if (From->Slots[i].Item)  // ← Item으로 수정
        {
            int32 Moved = 0;
            TransferFrom(From, i, From->Slots[i].Count, Moved);
            OutTotalMoved += Moved;
        }
    }

    if (OutTotalMoved > 0)
    {
        ScheduleSave();
    }

    return (OutTotalMoved > 0);
}

bool UInventoryComponent::DropAt(int32 FromIdx, int32 Count, const FTransform& WorldTransform, TSubclassOf<AItemPickup> PickupClass)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_DropAt(FromIdx, Count, WorldTransform, PickupClass);
        return false;
    }

    if (!ValidIndex(FromIdx) || !PickupClass)
    {
        return false;
    }

    FItemStack& Slot = Slots[FromIdx];
    if (!Slot.Item || Slot.Count < Count)  // ← Item으로 수정
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AItemPickup* Pickup = World->SpawnActor<AItemPickup>(PickupClass, WorldTransform, SpawnParams);
    if (Pickup)
    {
        Pickup->Stack.Item = Slot.Item;  // ← Item으로 수정
        Pickup->Stack.Count = Count;

        RemoveAt(FromIdx, Count);
        ScheduleSave();
        return true;
    }

    return false;
}

void UInventoryComponent::OnRep_Slots()
{
    BroadcastUpdated();
}

void UInventoryComponent::BroadcastUpdated()
{
    // UI 업데이트 등을 위한 델리게이트 호출
    // OnInventoryUpdated.Broadcast();
}

// ============================================================
// 저장 시스템
// ============================================================

void UInventoryComponent::ScheduleSave()
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    World->GetTimerManager().ClearTimer(SaveDebounceTimer);
    
    World->GetTimerManager().SetTimer(
        SaveDebounceTimer,
        this,
        &UInventoryComponent::SaveInventoryToServer,
        SaveDebounceDelay,
        false
    );
    
    UE_LOG(LogTemp, Verbose, TEXT("Save scheduled in %.1f seconds"), SaveDebounceDelay);
}

void UInventoryComponent::SaveInventoryToServer()
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return;
    }

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!PC || !PC->PlayerState)
    {
        return;
    }

    FString PlayerID;
    if (PC->PlayerState->GetUniqueId().IsValid())
    {
        PlayerID = PC->PlayerState->GetUniqueId()->ToString();
    }
    else
    {
        PlayerID = FString::Printf(TEXT("Local_%d"), PC->PlayerState->GetPlayerId());
    }

    TArray<FItemStack> EmptyStorage;
    if (UInventorySaveManager::SavePlayerInventory(this, PlayerID, Slots, EmptyStorage))
    {
        UE_LOG(LogTemp, Log, TEXT("⚡ Debounced save completed for: %s"), *PlayerID);
    }
}

void UInventoryComponent::ManualSave()
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(SaveDebounceTimer);
        }
    
        SaveInventoryToServer();
        UE_LOG(LogTemp, Log, TEXT("💾 Manual save triggered"));
    }
}


// ============================================================
// 서버 RPC 구현
// ============================================================

void UInventoryComponent::Server_TryAddStack_Implementation(const FItemStack& InStack)
{
    int32 Added = 0;
    TryAddStack(InStack, Added);
}

void UInventoryComponent::Server_TryMove_Implementation(int32 FromIdx, int32 ToIdx)
{
    TryMove(FromIdx, ToIdx);
}

void UInventoryComponent::Server_TrySplit_Implementation(int32 FromIdx, int32 NumToSplit, int32 ToIdx)
{
    TrySplit(FromIdx, NumToSplit, ToIdx);
}

void UInventoryComponent::Server_TransferFrom_Implementation(UInventoryComponent* From, int32 FromIdx, int32 Count)
{
    int32 Moved = 0;
    TransferFrom(From, FromIdx, Count, Moved);
}

void UInventoryComponent::Server_TransferAllFrom_Implementation(UInventoryComponent* From)
{
    int32 TotalMoved = 0;
    TransferAllFrom(From, TotalMoved);
}

void UInventoryComponent::Server_DropAt_Implementation(int32 FromIdx, int32 Count, const FTransform& Xform, TSubclassOf<AItemPickup> PickupClass)
{
    DropAt(FromIdx, Count, Xform, PickupClass);
}
