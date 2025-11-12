// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerLobbyManager.generated.h"

UCLASS()
class LOSTSECTOR_API UPlayerLobbyManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // 플레이어 개인 로비 입장
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void EnterPersonalLobby(const FString& PlayerID);

    // 필드맵으로 출격
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void DeployToField(const FString& MapName);

    // 로비에서 나가기
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void ExitLobby();

private:
    FString CurrentPlayerID;
};
