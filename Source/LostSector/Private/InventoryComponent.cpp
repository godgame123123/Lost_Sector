#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "ItemPickup.h"
#include "InventorySaveManager.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "ItemDataBase.h"
#include "Engine/AssetManager.h"
#include "../LostSectorCharacter.h"

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
        InitStorageSlots();
    }
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UInventoryComponent, Slots);
    DOREPLIFETIME(UInventoryComponent, StorageSlots);
    DOREPLIFETIME(UInventoryComponent, ExpandedSlotCount);
}

void UInventoryComponent::InitSlots()
{
    int32 TotalSlots = BaseSlotCount + ExpandedSlotCount;
    Slots.Empty();
    for (int32 i = 0; i < TotalSlots; i++)
    {
        Slots.Add(FItemStack());
    }

    // 초기 슬롯 생성 시에도 UI 갱신
    BroadcastUpdated();
}

void UInventoryComponent::InitStorageSlots()
{
    StorageSlots.Empty();
    const int32 StorageSlotCount = 30;
    for (int32 i = 0; i < StorageSlotCount; i++)
    {
        StorageSlots.Add(FItemStack());
    }
}

bool UInventoryComponent::ExpandInventoryWithBag(int32 AdditionalSlots)
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return false;
    }

    if (AdditionalSlots <= 0 || ExpandedSlotCount + AdditionalSlots > MaxExpandedSlots)
    {
        return false;
    }

    ExpandedSlotCount += AdditionalSlots;
    
    // 기존 슬롯 유지하면서 새 슬롯 추가
    int32 CurrentCount = Slots.Num();
    int32 NewCount = BaseSlotCount + ExpandedSlotCount;
    
    // 슬롯 확장
    for (int32 i = CurrentCount; i < NewCount; i++)
    {
        Slots.Add(FItemStack());
    }

    BroadcastUpdated();
    ScheduleSave();
    return true;
}

bool UInventoryComponent::ShrinkInventory(int32 SlotsToRemove)
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return false;
    }

    if (SlotsToRemove <= 0 || ExpandedSlotCount < SlotsToRemove)
    {
        return false;
    }

    int32 NewExpandedCount = ExpandedSlotCount - SlotsToRemove;
    int32 NewTotalSlots = BaseSlotCount + NewExpandedCount;
    
    // 확장 슬롯에 아이템이 있는지 확인
    int32 CurrentTotalSlots = Slots.Num();
    for (int32 i = NewTotalSlots; i < CurrentTotalSlots; i++)
    {
        if (Slots[i].Item && Slots[i].Count > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot shrink inventory: items in expanded slots"));
            return false;
        }
    }

    ExpandedSlotCount = NewExpandedCount;
    Slots.SetNum(NewTotalSlots);

    BroadcastUpdated();
    ScheduleSave();
    return true;
}

bool UInventoryComponent::TryAddToStorage(const FItemStack& InStack, int32& OutAdded)
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        OutAdded = 0;
        return false;
    }

    if (!InStack.Item || InStack.Count <= 0)
    {
        OutAdded = 0;
        return false;
    }

    int32 Remaining = InStack.Count;
    OutAdded = 0;

    // 1단계: 기존 스택에 추가
    for (int32 i = 0; i < StorageSlots.Num() && Remaining > 0; i++)
    {
        FItemStack& Slot = StorageSlots[i];
        if (Slot.Item == InStack.Item && Slot.Count < InStack.Item->MaxStack)
        {
            int32 CanAdd = FMath::Min(Remaining, InStack.Item->MaxStack - Slot.Count);
            Slot.Count += CanAdd;
            Remaining -= CanAdd;
            OutAdded += CanAdd;
        }
    }

    // 2단계: 빈 슬롯에 추가
    for (int32 i = 0; i < StorageSlots.Num() && Remaining > 0; i++)
    {
        FItemStack& Slot = StorageSlots[i];
        if (!Slot.Item)
        {
            int32 CanAdd = FMath::Min(Remaining, InStack.Item->MaxStack);
            Slot.Item = InStack.Item;
            Slot.ItemId = InStack.Item ? InStack.Item->ItemId : InStack.ItemId;
            Slot.Count = CanAdd;
            Remaining -= CanAdd;
            OutAdded += CanAdd;
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

bool UInventoryComponent::RemoveFromStorage(int32 Index, int32 Count)
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return false;
    }

    if (!ValidStorageIndex(Index))
    {
        return false;
    }

    FItemStack& Slot = StorageSlots[Index];
    if (!Slot.Item || Slot.Count < Count)
    {
        return false;
    }

    Slot.Count -= Count;
    if (Slot.Count <= 0)
    {
        Slot.Item = nullptr;
        Slot.ItemId = NAME_None;
        Slot.Count = 0;
    }

    BroadcastUpdated();
    ScheduleSave();
    return true;
}

bool UInventoryComponent::TryMoveStorage(int32 FromIdx, int32 ToIdx)
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return false;
    }

    if (!ValidStorageIndex(FromIdx) || !ValidStorageIndex(ToIdx) || FromIdx == ToIdx)
    {
        return false;
    }

    FItemStack Temp = StorageSlots[FromIdx];
    StorageSlots[FromIdx] = StorageSlots[ToIdx];
    StorageSlots[ToIdx] = Temp;

    BroadcastUpdated();
    ScheduleSave();
    return true;
}

float UInventoryComponent::GetTotalWeight() const
{
    float Total = 0.f;
    for (const FItemStack& Stack : Slots)
    {
        if (Stack.Item)
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
bool UInventoryComponent::CanAddItem(UItemDataBase* ItemData, int32 Count)
{
    if (!ItemData || Count <= 0) return false;

    // 1. 무게 체크 (위의 float 버전 CanAddWeight 함수 사용)
    float WeightToAdd = ItemData->Weight * Count;
    if (!CanAddWeight(WeightToAdd)) return false;

    // 2. 공간(슬롯) 체크
    int32 RemainingCount = Count;

    // A. 겹쳐지기(Stack) 확인
    for (const FItemStack& Slot : Slots)
    {
        if (Slot.Item == ItemData && Slot.Count < ItemData->MaxStack)
        {
            int32 SpaceInSlot = ItemData->MaxStack - Slot.Count;
            RemainingCount -= SpaceInSlot;
            if (RemainingCount <= 0) return true;
        }
    }

    // B. 빈 슬롯 확인
    for (const FItemStack& Slot : Slots)
    {
        if (Slot.Item == nullptr)
        {
            RemainingCount -= ItemData->MaxStack;
            if (RemainingCount <= 0) return true;
        }
    }

    return false; // 공간 부족
}

int32 UInventoryComponent::AddItem(UItemDataBase* ItemData, int32 Count)
{
    if (!ItemData || Count <= 0) return 0;

    FItemStack NewStack;
    NewStack.Item = ItemData;
    NewStack.ItemId = ItemData->ItemId;
    NewStack.Count = Count;

    int32 AddedAmount = 0;
    TryAddStack(NewStack, AddedAmount);

    return AddedAmount;
}

bool UInventoryComponent::TryAddStack(const FItemStack& InStack, int32& OutAdded)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_TryAddStack(InStack);
        return false;
    }

    if (!InStack.Item || InStack.Count <= 0)
    {
        OutAdded = 0;
        return false;
    }

    float SingleWeight = InStack.Item->Weight;
    int32 Remaining = InStack.Count;
    OutAdded = 0;

    // 1단계: 기존 스택에 추가
    for (int32 i = 0; i < Slots.Num() && Remaining > 0; i++)
    {
        FItemStack& Slot = Slots[i];
        if (Slot.Item == InStack.Item && Slot.Count < InStack.Item->MaxStack)
        {
            int32 CanAdd = FMath::Min(Remaining, InStack.Item->MaxStack - Slot.Count);
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
        if (!Slot.Item)
        {
            int32 CanAdd = FMath::Min(Remaining, InStack.Item->MaxStack);
            if (CanAddWeight(CanAdd * SingleWeight))
            {
                Slot.Item = InStack.Item;
                Slot.ItemId = InStack.Item ? InStack.Item->ItemId : InStack.ItemId;
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

    if (!From.Item || From.Count < NumToSplit || NumToSplit <= 0)
    {
        return false;
    }

    if (To.Item)
    {
        return false;
    }

    To.Item = From.Item;
    To.Count = NumToSplit;
    From.Count -= NumToSplit;

    if (From.Count <= 0)
    {
        From.Item = nullptr;
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
    if (!Slot.Item || Slot.Count < Count)
    {
        return false;
    }

    Slot.Count -= Count;
    if (Slot.Count <= 0)
    {
        Slot.Item = nullptr;
        Slot.ItemId = NAME_None;
        Slot.Count = 0;
    }

    BroadcastUpdated();
    ScheduleSave();
    return true;
}

bool UInventoryComponent::UseItem(int32 Index)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_UseItem(Index);
        return false;
    }

    if (!ValidIndex(Index))
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Invalid index %d"), Index);
        return false;
    }

    FItemStack& Slot = Slots[Index];
    if (!Slot.Item || Slot.Count <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: No item at index %d"), Index);
        return false;
    }

    // 소비 가능한 아이템인지 확인
    if (!Slot.Item->IsConsumable())
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Item %s is not consumable"), *Slot.Item->DisplayName.ToString());
        return false;
    }

    // 캐릭터 가져오기
    ALostSectorCharacter* Character = Cast<ALostSectorCharacter>(GetOwner());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Owner is not a LostSectorCharacter"));
        return false;
    }

    // 아이템 효과 적용
    bool bEffectApplied = false;
    
    if (Slot.Item->HealAmount > 0.f)
    {
        float MaxHp = 100.f; // TODO: 최대 체력을 캐릭터에서 가져오기
        Character->CharacterStats.Hp = FMath::Min(MaxHp, Character->CharacterStats.Hp + Slot.Item->HealAmount);
        bEffectApplied = true;
        UE_LOG(LogTemp, Log, TEXT("✅ Healed %f HP. Current HP: %f"), Slot.Item->HealAmount, Character->CharacterStats.Hp);
    }

    if (Slot.Item->StaminaAmount > 0.f)
    {
        float MaxStamina = 100.f; // TODO: 최대 스태미나를 캐릭터에서 가져오기
        Character->CharacterStats.Stamina = FMath::Min(MaxStamina, Character->CharacterStats.Stamina + Slot.Item->StaminaAmount);
        bEffectApplied = true;
        UE_LOG(LogTemp, Log, TEXT("✅ Restored %f Stamina. Current Stamina: %f"), Slot.Item->StaminaAmount, Character->CharacterStats.Stamina);
    }

    if (Slot.Item->HungerAmount > 0.f)
    {
        float MaxHunger = 100.f; // TODO: 최대 배고픔을 캐릭터에서 가져오기
        Character->CharacterStats.hungry = FMath::Min(MaxHunger, Character->CharacterStats.hungry + Slot.Item->HungerAmount);
        bEffectApplied = true;
        UE_LOG(LogTemp, Log, TEXT("✅ Restored %f Hunger. Current Hunger: %f"), Slot.Item->HungerAmount, Character->CharacterStats.hungry);
    }

    if (!bEffectApplied)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Item has no effect"));
        return false;
    }

    // 아이템 개수 감소 (1개 사용)
    Slot.Count -= 1;
    if (Slot.Count <= 0)
    {
        Slot.Item = nullptr;
        Slot.ItemId = NAME_None;
        Slot.Count = 0;
    }

    BroadcastUpdated();
    ScheduleSave(); // JSON 파일에 자동 저장
    UE_LOG(LogTemp, Log, TEXT("✅ Item used. Remaining count: %d"), Slot.Count);
    
    return true;
}

int32 UInventoryComponent::GetItemCountByItemId(FName ItemId) const
{
    int32 TotalCount = 0;
    
    for (const FItemStack& Slot : Slots)
    {
        if (Slot.ItemId == ItemId && Slot.Count > 0)
        {
            TotalCount += Slot.Count;
        }
    }
    
    return TotalCount;
}

int32 UInventoryComponent::GetItemCountByItemData(UItemDataBase* ItemData) const
{
    if (!ItemData)
    {
        return 0;
    }
    
    return GetItemCountByItemId(ItemData->ItemId);
}

int32 UInventoryComponent::RemoveItemByItemData(UItemDataBase* ItemData, int32 Count)
{
    // 서버에서만 실행되도록 권한 체크
    if (GetOwnerRole() != ROLE_Authority)
    {
        return 0;
    }

    if (!ItemData || Count <= 0)
    {
        return 0;
    }

    int32 RemainingToRemove = Count;
    int32 RemovedTotal = 0;

    // 인벤토리 슬롯을 순회하며 아이템 제거
    for (int32 i = 0; i < Slots.Num() && RemainingToRemove > 0; i++)
    {
        FItemStack& Slot = Slots[i];

        // Item 포인터 또는 ItemId가 일치하는지 확인
        if (Slot.Item == ItemData || Slot.ItemId == ItemData->ItemId)
        {
            // 이 슬롯에서 제거할 수 있는 최대 수량 계산
            int32 ToRemove = FMath::Min(RemainingToRemove, Slot.Count);

            Slot.Count -= ToRemove;
            RemainingToRemove -= ToRemove;
            RemovedTotal += ToRemove;

            // 슬롯의 아이템이 0개가 되면 슬롯을 비웁니다.
            if (Slot.Count <= 0)
            {
                Slot.Item = nullptr;
                Slot.ItemId = NAME_None;
                Slot.Count = 0;
            }
        }
    }

    // 실제로 제거된 아이템이 있다면 UI 갱신 및 저장 스케줄링
    if (RemovedTotal > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Removed %d of ItemId: %s from inventory."), RemovedTotal, *ItemData->ItemId.ToString());
        BroadcastUpdated();
        ScheduleSave();
    }

    // 요청된 수량(Count)과 실제 제거된 수량(RemovedTotal)이 다를 경우 로그 경고를 남길 수도 있습니다.
    if (RemainingToRemove > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveItemByItemData: Could not remove full amount. Requested: %d, Removed: %d"), Count, RemovedTotal);
    }

    return RemovedTotal;
}

bool UInventoryComponent::ConsumeAmmo(UItemDataBase* AmmoItemData, int32 Amount)
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return false;
    }

    if (!AmmoItemData || Amount <= 0)
    {
        return false;
    }

    int32 Remaining = Amount;

    // 인벤토리에서 총알 찾아서 소비
    for (int32 i = 0; i < Slots.Num() && Remaining > 0; i++)
    {
        FItemStack& Slot = Slots[i];
        if (Slot.Item == AmmoItemData || Slot.ItemId == AmmoItemData->ItemId)
        {
            int32 ToConsume = FMath::Min(Remaining, Slot.Count);
            Slot.Count -= ToConsume;
            Remaining -= ToConsume;

            if (Slot.Count <= 0)
            {
                Slot.Item = nullptr;
                Slot.ItemId = NAME_None;
                Slot.Count = 0;
            }
        }
    }

    if (Remaining > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConsumeAmmo: Not enough ammo. Requested: %d, Consumed: %d"), Amount, Amount - Remaining);
        return false;
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
    if (!FromSlot.Item || FromSlot.Count < Count)
    {
        return false;
    }

    FItemStack TransferStack;
    TransferStack.Item = FromSlot.Item;
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
        if (From->Slots[i].Item)
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
    if (!Slot.Item || Slot.Count < Count)
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
        Pickup->Stack.Item = Slot.Item;
        Pickup->Stack.Count = Count;

        RemoveAt(FromIdx, Count);
        ScheduleSave();
        return true;
    }

    return false;
}

// =============================
// RepNotify + 델리게이트
// =============================

void UInventoryComponent::OnRep_Slots()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_Slots called on client"));
    // 클라에서 Slots 복제될 때 Item 포인터가 nullptr일 수 있으므로 ItemId로 복원
    RestoreItemPointers();
    BroadcastUpdated();
}

void UInventoryComponent::OnRep_StorageSlots()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_StorageSlots called on client"));
    // 클라에서 StorageSlots 복제될 때 Item 포인터 복원
    RestoreStorageItemPointers();
    BroadcastUpdated();
}

void UInventoryComponent::RestoreItemPointers()
{
    UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
    if (!AssetManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("RestoreItemPointers: AssetManager not available"));
        return;
    }

    int32 RestoredCount = 0;
    for (FItemStack& Slot : Slots)
    {
        // Item 포인터가 nullptr이고 ItemId가 유효한 경우 복원
        if (!Slot.Item && Slot.ItemId != NAME_None)
        {
            // PrimaryDataAsset을 ItemId로 로드
            FPrimaryAssetType PrimaryAssetType = UItemDataBase::StaticClass()->GetFName();
            FPrimaryAssetId PrimaryAssetId = FPrimaryAssetId(PrimaryAssetType, Slot.ItemId);
            UItemDataBase* LoadedItem = Cast<UItemDataBase>(AssetManager->GetPrimaryAssetObject(PrimaryAssetId));
            
            if (LoadedItem)
            {
                Slot.Item = LoadedItem;
                RestoredCount++;
                UE_LOG(LogTemp, Log, TEXT("✅ Restored item pointer for ItemId: %s"), *Slot.ItemId.ToString());
            }
            else
            {
                // 동기 로드 시도
                TSharedPtr<FStreamableHandle> Handle = AssetManager->LoadPrimaryAsset(PrimaryAssetId, TArray<FName>());
                if (Handle.IsValid())
                {
                    Handle->WaitUntilComplete();
                    LoadedItem = Cast<UItemDataBase>(AssetManager->GetPrimaryAssetObject(PrimaryAssetId));
                    if (LoadedItem)
                    {
                        Slot.Item = LoadedItem;
                        RestoredCount++;
                        UE_LOG(LogTemp, Log, TEXT("✅ Restored item pointer (sync load) for ItemId: %s"), *Slot.ItemId.ToString());
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("❌ Failed to restore item pointer for ItemId: %s"), *Slot.ItemId.ToString());
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("❌ Failed to create load handle for ItemId: %s"), *Slot.ItemId.ToString());
                }
            }
        }
    }
    
    if (RestoredCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Restored %d item pointers"), RestoredCount);
    }
}

void UInventoryComponent::RestoreStorageItemPointers()
{
    UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
    if (!AssetManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("RestoreStorageItemPointers: AssetManager not available"));
        return;
    }

    int32 RestoredCount = 0;
    for (FItemStack& Slot : StorageSlots)
    {
        // Item 포인터가 nullptr이고 ItemId가 유효한 경우 복원
        if (!Slot.Item && Slot.ItemId != NAME_None)
        {
            // PrimaryDataAsset을 ItemId로 로드
            FPrimaryAssetType PrimaryAssetType = UItemDataBase::StaticClass()->GetFName();
            FPrimaryAssetId PrimaryAssetId = FPrimaryAssetId(PrimaryAssetType, Slot.ItemId);
            UItemDataBase* LoadedItem = Cast<UItemDataBase>(AssetManager->GetPrimaryAssetObject(PrimaryAssetId));
            
            if (LoadedItem)
            {
                Slot.Item = LoadedItem;
                RestoredCount++;
                UE_LOG(LogTemp, Log, TEXT("✅ Restored storage item pointer for ItemId: %s"), *Slot.ItemId.ToString());
            }
            else
            {
                // 동기 로드 시도
                TSharedPtr<FStreamableHandle> Handle = AssetManager->LoadPrimaryAsset(PrimaryAssetId, TArray<FName>());
                if (Handle.IsValid())
                {
                    Handle->WaitUntilComplete();
                    LoadedItem = Cast<UItemDataBase>(AssetManager->GetPrimaryAssetObject(PrimaryAssetId));
                    if (LoadedItem)
                    {
                        Slot.Item = LoadedItem;
                        RestoredCount++;
                        UE_LOG(LogTemp, Log, TEXT("✅ Restored storage item pointer (sync load) for ItemId: %s"), *Slot.ItemId.ToString());
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("❌ Failed to restore storage item pointer for ItemId: %s"), *Slot.ItemId.ToString());
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("❌ Failed to create load handle for storage ItemId: %s"), *Slot.ItemId.ToString());
                }
            }
        }
    }
    
    if (RestoredCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Restored %d storage item pointers"), RestoredCount);
    }
}

void UInventoryComponent::BroadcastUpdated()
{
    UE_LOG(LogTemp, Warning, TEXT("BroadcastUpdated -> OnInventoryUpdated.Broadcast()"));
    OnInventoryUpdated.Broadcast();
}

// =============================
// 저장 시스템
// =============================

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

    // ItemId 설정 (저장 전에 Item 포인터에서 ItemId 추출)
    for (FItemStack& Stack : Slots)
    {
        if (Stack.Item && Stack.ItemId == NAME_None)
        {
            Stack.ItemId = Stack.Item->ItemId;
        }
    }
    for (FItemStack& Stack : StorageSlots)
    {
        if (Stack.Item && Stack.ItemId == NAME_None)
        {
            Stack.ItemId = Stack.Item->ItemId;
        }
    }

    if (UInventorySaveManager::SavePlayerInventory(this, PlayerID, Slots, StorageSlots))
    {
        UE_LOG(LogTemp, Log, TEXT("⚡ Debounced save completed for: %s (Inventory: %d, Storage: %d)"), 
            *PlayerID, Slots.Num(), StorageSlots.Num());
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

void UInventoryComponent::DebugPrintInventory() const
{
    int32 ValidSlots = 0;
    float TotalWeight = 0.f;
    
    UE_LOG(LogTemp, Warning, TEXT("=== 인벤토리 상태 ==="));
    UE_LOG(LogTemp, Warning, TEXT("총 슬롯 수: %d"), Slots.Num());
    
    for (int32 i = 0; i < Slots.Num(); i++)
    {
        const FItemStack& Slot = Slots[i];
        if (Slot.Item && Slot.Count > 0)
        {
            ValidSlots++;
            TotalWeight += Slot.Item->Weight * Slot.Count;
            UE_LOG(LogTemp, Warning, TEXT("  Slot[%d]: ItemId=%s, Count=%d, Weight=%.2f"), 
                i, 
                Slot.ItemId != NAME_None ? *Slot.ItemId.ToString() : TEXT("NULL"),
                Slot.Count,
                Slot.Item->Weight * Slot.Count);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("유효 슬롯: %d / %d"), ValidSlots, Slots.Num());
    UE_LOG(LogTemp, Warning, TEXT("총 무게: %.2f / %.2f"), TotalWeight, WeightLimit);
    UE_LOG(LogTemp, Warning, TEXT("===================="));
}

// =============================
// 서버 RPC 구현
// =============================

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

void UInventoryComponent::Server_UseItem_Implementation(int32 Index)
{
    UseItem(Index);
}
