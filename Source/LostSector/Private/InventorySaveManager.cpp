// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySaveManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "InventoryComponent.h"

FString UInventorySaveManager::GetSaveFilePath(const FString& PlayerID)
{
    FString SaveDir = FPaths::ProjectSavedDir() + TEXT("ServerInventoryData/");
    FString SafePlayerID = PlayerID.Replace(TEXT(":"), TEXT("_")).Replace(TEXT("/"), TEXT("_"));
    return SaveDir + FString::Printf(TEXT("Inv_%s.json"), *SafePlayerID);
}

bool UInventorySaveManager::SavePlayerInventory(UObject* WorldContextObject, const FString& PlayerID,
    const TArray<FItemStack>& InventorySlots, const TArray<FItemStack>& StorageSlots)
{
    if (!WorldContextObject)
    {
        return false;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World || !World->GetAuthGameMode())
    {
        UE_LOG(LogTemp, Warning, TEXT("SavePlayerInventory: Server only"));
        return false;
    }

    if (PlayerID.IsEmpty())
    {
        return false;
    }

    FPlayerInventorySaveData SaveData;
    SaveData.PlayerID = PlayerID;
    SaveData.InventorySlots = InventorySlots;
    SaveData.StorageSlots = StorageSlots;
    SaveData.LastSaveTime = FDateTime::Now();

    FString JsonString;
    if (!FJsonObjectConverter::UStructToJsonObjectString(SaveData, JsonString, 0, 0, 0, nullptr, true))
    {
        return false;
    }

    FString FilePath = GetSaveFilePath(PlayerID);
    FString Directory = FPaths::GetPath(FilePath);
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*Directory))
    {
        PlatformFile.CreateDirectoryTree(*Directory);
    }

    if (FFileHelper::SaveStringToFile(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("✅ Inventory saved: %s"), *PlayerID);
        return true;
    }

    return false;
}

bool UInventorySaveManager::LoadPlayerInventory(UObject* WorldContextObject, const FString& PlayerID,
    FPlayerInventorySaveData& OutData)
{
    if (!WorldContextObject)
    {
        return false;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World || !World->GetAuthGameMode())
    {
        return false;
    }

    FString FilePath = GetSaveFilePath(PlayerID);
    if (!FPaths::FileExists(FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("No save file for: %s"), *PlayerID);
        return false;
    }

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        return false;
    }

    if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutData, 0, 0))
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("✅ Inventory loaded: %s"), *PlayerID);
    return true;
}

void UInventorySaveManager::EnableAutoSave(float IntervalSeconds)
{
    UWorld* World = GetWorld();
    if (!World || !World->GetAuthGameMode())
    {
        return;
    }

    DisableAutoSave();
    World->GetTimerManager().SetTimer(AutoSaveTimerHandle, this, 
        &UInventorySaveManager::AutoSaveAllPlayers, IntervalSeconds, true);
}

void UInventorySaveManager::DisableAutoSave()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
    }
}

void UInventorySaveManager::AutoSaveAllPlayers()
{
    UWorld* World = GetWorld();
    if (!World || !World->GetAuthGameMode())
    {
        return;
    }

    AGameStateBase* GameState = World->GetGameState();
    if (!GameState)
    {
        return;
    }

    int32 SavedCount = 0;
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        if (!PlayerState || !PlayerState->GetPawn())
        {
            continue;
        }

        FString PlayerID = PlayerState->GetUniqueId().IsValid()
            ? PlayerState->GetUniqueId()->ToString()
            : FString::Printf(TEXT("Local_%d"), PlayerState->GetPlayerId());

        UInventoryComponent* InventoryComp = PlayerState->GetPawn()->FindComponentByClass<UInventoryComponent>();
        if (InventoryComp)
        {
            if (SavePlayerInventory(this, PlayerID, InventoryComp->Slots, TArray<FItemStack>()))
            {
                SavedCount++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Auto-saved %d players"), SavedCount);
}
