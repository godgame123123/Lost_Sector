// Fill out your copyright notice in the Description page of Project Settings.


// LobbyPlayerController.cpp
#include "LobbyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
/*
void ALobbyPlayerController::InitializePlayerData(const FPlayerData& Data)
{
    CurrentPlayerData = Data;
    ClientUpdateUI();
}

void ALobbyPlayerController::ServerBuyItem_Implementation(const FString& ItemID, int32 Price)
{
    // 돈이 충분한지 확인
    if (CurrentPlayerData.Money >= Price)
    {
        CurrentPlayerData.Money -= Price;
        CurrentPlayerData.Inventory.Add(ItemID);
        
        UE_LOG(LogTemp, Log, TEXT("Player bought %s for %d. Remaining money: %d"), 
               *ItemID, Price, CurrentPlayerData.Money);
        
        ClientUpdateUI();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough money to buy %s"), *ItemID);
    }
}

void ALobbyPlayerController::ServerSellItem_Implementation(const FString& ItemID, int32 Price)
{
    if (CurrentPlayerData.Inventory.Contains(ItemID))
    {
        CurrentPlayerData.Inventory.Remove(ItemID);
        CurrentPlayerData.Money += Price;
        
        UE_LOG(LogTemp, Log, TEXT("Player sold %s for %d. Total money: %d"), 
               *ItemID, Price, CurrentPlayerData.Money);
        
        ClientUpdateUI();
    }
}

void ALobbyPlayerController::ServerCraftItem_Implementation(const FString& RecipeID)
{
    // 여기서 레시피 확인 및 재료 소모 로직
    // 예시: 간단한 제작
    if (CurrentPlayerData.Inventory.Contains("Material_Wood") && 
        CurrentPlayerData.Inventory.Contains("Material_Iron"))
    {
        CurrentPlayerData.Inventory.Remove("Material_Wood");
        CurrentPlayerData.Inventory.Remove("Material_Iron");
        CurrentPlayerData.Inventory.Add("Weapon_Sword");
        
        UE_LOG(LogTemp, Log, TEXT("Crafted item: %s"), *RecipeID);
        ClientUpdateUI();
    }
}

void ALobbyPlayerController::ServerMoveItem_Implementation(const FString& ItemID, int32 FromSlot, int32 ToSlot)
{
    // 인벤토리 슬롯 이동 로직
    UE_LOG(LogTemp, Log, TEXT("Moved %s from slot %d to %d"), *ItemID, FromSlot, ToSlot);
}

void ALobbyPlayerController::ServerStartRaid_Implementation(const FString& MapName)
{
    // 필드맵으로 이동 (여러 플레이어 가능)
    GetWorld()->ServerTravel(MapName + "?listen");
}

void ALobbyPlayerController::ClientUpdateUI_Implementation()
{
    // 블루프린트에서 구현 (UI 갱신)
    UE_LOG(LogTemp, Log, TEXT("UI Updated - Money: %d, Items: %d"), 
           CurrentPlayerData.Money, CurrentPlayerData.Inventory.Num());
}

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALobbyPlayerController, CurrentPlayerData);
}

*/