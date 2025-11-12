// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ServerDataManager.h"
#include "LobbyPlayerController.generated.h"

UCLASS()
class LOSTSECTOR_API ALobbyPlayerController : public APlayerController
{
    GENERATED_BODY()
/*
public:
    // 플레이어 데이터 초기화
    void InitializePlayerData(const FPlayerData& Data);
    
    // 현재 데이터 가져오기
    FPlayerData GetCurrentPlayerData() const { return CurrentPlayerData; }

    // === 상점 기능 ===
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Shop")
    void ServerBuyItem(const FString& ItemID, int32 Price);
    
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Shop")
    void ServerSellItem(const FString& ItemID, int32 Price);

    // === 제작대 기능 ===
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Crafting")
    void ServerCraftItem(const FString& RecipeID);

    // === 인벤토리 기능 ===
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory")
    void ServerMoveItem(const FString& ItemID, int32 FromSlot, int32 ToSlot);

    // === 필드맵 이동 ===
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Lobby")
    void ServerStartRaid(const FString& MapName);

protected:
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player Data")
    FPlayerData CurrentPlayerData;

    // UI 업데이트 (클라이언트)
    UFUNCTION(Client, Reliable)
    void ClientUpdateUI();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;*/
};
