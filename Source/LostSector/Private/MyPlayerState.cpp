#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "ServerDataManager.h"

AMyPlayerState::AMyPlayerState()
{
    Money = 0;
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AMyPlayerState, PlayerData);
    DOREPLIFETIME(AMyPlayerState, StashItems);
    DOREPLIFETIME(AMyPlayerState, InventoryItems);
    DOREPLIFETIME(AMyPlayerState, Money);
}

void AMyPlayerState::BeginPlay()
{
    Super::BeginPlay();
    
    // 서버에서만 실행
    if (HasAuthority())
    {
        LoadPlayerDataFromServer();
    }
}

void AMyPlayerState::LoadPlayerDataFromServer()
{
    if (!HasAuthority()) return;
    
    // PlayerID는 Steam ID나 고유 ID 사용
    FString PlayerID = GetUniqueId().IsValid() 
        ? GetUniqueId()->ToString() 
        : FString::Printf(TEXT("Local_%d"), GetPlayerId());
    
    UServerDataManager* DataManager = UServerDataManager::GetInstance(GetWorld());
    if (DataManager)
    {
        FPlayerData LoadedData;
        if (DataManager->LoadPlayerData(PlayerID, LoadedData))
        {
            StashItems = LoadedData.StashItems;
            InventoryItems = LoadedData.InventoryItems;
            Money = LoadedData.Money;
            
            UE_LOG(LogTemp, Log, TEXT("Loaded player data: %s (Money: %d, Stash: %d items)"), 
                *PlayerID, Money, StashItems.Num());
        }
        else
        {
            // 신규 플레이어 - 기본값 설정
            Money = 10000;
            UE_LOG(LogTemp, Log, TEXT("New player: %s (Starting money: %d)"), *PlayerID, Money);
        }
    }
}

void AMyPlayerState::SavePlayerDataToServer()
{
    if (!HasAuthority()) return;
    
    FString PlayerID = GetUniqueId().IsValid() 
        ? GetUniqueId()->ToString() 
        : FString::Printf(TEXT("Local_%d"), GetPlayerId());
    
    UServerDataManager* DataManager = UServerDataManager::GetInstance(GetWorld());
    if (DataManager)
    {
        FPlayerData DataToSave;
        DataToSave.StashItems = StashItems;
        DataToSave.InventoryItems = InventoryItems;
        DataToSave.Money = Money;
        
        if (DataManager->SavePlayerData(PlayerID, DataToSave))
        {
            UE_LOG(LogTemp, Log, TEXT("Saved player data: %s"), *PlayerID);
        }
    }
}

void AMyPlayerState::Server_AddItemToStash_Implementation(const FItemStack& ItemStack)
{
    if (ItemStack.IsValid())
    {
        StashItems.Add(ItemStack);
        SavePlayerDataToServer(); // 즉시 저장
        
        UE_LOG(LogTemp, Log, TEXT("Added item to stash: %s x%d"), 
            *ItemStack.Item->GetName(), ItemStack.Count);
    }
}

bool AMyPlayerState::Server_AddItemToStash_Validate(const FItemStack& ItemStack)
{
    // 치팅 검증
    return ItemStack.IsValid();
}

void AMyPlayerState::Server_RemoveItemFromStash_Implementation(int32 Index)
{
    if (StashItems.IsValidIndex(Index))
    {
        FItemStack RemovedItem = StashItems[Index];
        StashItems.RemoveAt(Index);
        SavePlayerDataToServer(); // 즉시 저장
        
        UE_LOG(LogTemp, Log, TEXT("Removed item from stash at index %d"), Index);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid stash index: %d"), Index);
    }
}

bool AMyPlayerState::Server_RemoveItemFromStash_Validate(int32 Index)
{
    // 치팅 검증: 인덱스가 유효한 범위인지 확인
    return Index >= 0 && Index < 1000; // 최대 슬롯 수를 넘지 않는지 확인
}

void AMyPlayerState::OnRep_StashItems()
{
    // UI 업데이트
    UE_LOG(LogTemp, Log, TEXT("Stash updated: %d items"), StashItems.Num());
}

void AMyPlayerState::OnRep_InventoryItems()
{
    // UI 업데이트
    UE_LOG(LogTemp, Log, TEXT("Inventory updated: %d items"), InventoryItems.Num());
}

void AMyPlayerState::OnRep_Money()
{
    // UI 업데이트
    UE_LOG(LogTemp, Log, TEXT("Money updated: %d"), Money);
}

void AMyPlayerState::UpgradeStat(EStatTypes Stat, int32 Amount)
{
    // 기존 코드 유지
}
