// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySaveManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "InventoryComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"

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

void UInventorySaveManager::DebugPrintSaveFile(UObject* WorldContextObject, const FString& PlayerID)
{
    if (!WorldContextObject)
    {
        return;
    }

    FString FilePath = GetSaveFilePath(PlayerID);
    UE_LOG(LogTemp, Warning, TEXT("=== 저장 파일 정보 ==="));
    UE_LOG(LogTemp, Warning, TEXT("PlayerID: %s"), *PlayerID);
    UE_LOG(LogTemp, Warning, TEXT("파일 경로: %s"), *FilePath);
    UE_LOG(LogTemp, Warning, TEXT("파일 존재: %s"), FPaths::FileExists(FilePath) ? TEXT("YES") : TEXT("NO"));

    if (FPaths::FileExists(FilePath))
    {
        FString JsonString;
        if (FFileHelper::LoadFileToString(JsonString, *FilePath))
        {
            FPlayerInventorySaveData LoadedData;
            if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &LoadedData, 0, 0))
            {
                UE_LOG(LogTemp, Warning, TEXT("인벤토리 슬롯 수: %d"), LoadedData.InventorySlots.Num());
                UE_LOG(LogTemp, Warning, TEXT("창고 슬롯 수: %d"), LoadedData.StorageSlots.Num());
                UE_LOG(LogTemp, Warning, TEXT("마지막 저장 시간: %s"), *LoadedData.LastSaveTime.ToString());
                
                int32 ValidInventoryItems = 0;
                for (const FItemStack& Stack : LoadedData.InventorySlots)
                {
                    if (Stack.ItemId != NAME_None && Stack.Count > 0)
                    {
                        ValidInventoryItems++;
                    }
                }
                
                int32 ValidStorageItems = 0;
                for (const FItemStack& Stack : LoadedData.StorageSlots)
                {
                    if (Stack.ItemId != NAME_None && Stack.Count > 0)
                    {
                        ValidStorageItems++;
                    }
                }
                
                UE_LOG(LogTemp, Warning, TEXT("유효 인벤토리 아이템: %d"), ValidInventoryItems);
                UE_LOG(LogTemp, Warning, TEXT("유효 창고 아이템: %d"), ValidStorageItems);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("JSON 파싱 실패"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("파일 읽기 실패"));
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("===================="));
}

void UInventorySaveManager::DebugListAllSaveFiles(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return;
    }

    FString SaveDir = FPaths::ProjectSavedDir() + TEXT("ServerInventoryData/");
    UE_LOG(LogTemp, Warning, TEXT("=== 모든 저장 파일 목록 ==="));
    UE_LOG(LogTemp, Warning, TEXT("저장 디렉토리: %s"), *SaveDir);

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*SaveDir))
    {
        UE_LOG(LogTemp, Warning, TEXT("디렉토리가 존재하지 않습니다."));
        return;
    }

    TArray<FString> Files;
    PlatformFile.FindFiles(Files, *SaveDir, TEXT("*.json"));
    
    UE_LOG(LogTemp, Warning, TEXT("찾은 파일 수: %d"), Files.Num());
    for (const FString& File : Files)
    {
        FString FullPath = SaveDir + File;
        // GetTimeStamp는 파일 경로만 받고 FDateTime을 반환
        FDateTime FileTime = PlatformFile.GetTimeStamp(*FullPath);
        if (FileTime.GetTicks() > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s (수정: %s)"), *File, *FileTime.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *File);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("===================="));
}
