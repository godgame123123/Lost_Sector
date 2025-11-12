// Fill out your copyright notice in the Description page of Project Settings.


// PlayerLobbyManager.cpp
#include "PlayerLobbyManager.h"
#include "Kismet/GameplayStatics.h"
#include "ServerDataManager.h"

void UPlayerLobbyManager::EnterPersonalLobby(const FString& PlayerID)
{
    CurrentPlayerID = PlayerID;
    
    // 플레이어 데이터 로드
    FPlayerData PlayerData;
    UServerDataManager::GetInstance(GetWorld())->LoadPlayerData(PlayerID, PlayerData);
    
    // 개인 로비맵으로 이동 (Listen Server로 시작)
    FString LobbyURL = FString::Printf(TEXT("/Game/Maps/PersonalLobby?listen?PlayerID=%s"), *PlayerID);
    UGameplayStatics::OpenLevel(GetWorld(), FName(*LobbyURL));
}

void UPlayerLobbyManager::DeployToField(const FString& MapName)
{
    // 필드맵은 멀티플레이어 가능
    FString FieldURL = FString::Printf(TEXT("%s?listen?PlayerID=%s"), *MapName, *CurrentPlayerID);
    UGameplayStatics::OpenLevel(GetWorld(), FName(*FieldURL));
}

void UPlayerLobbyManager::ExitLobby()
{
    // 데이터 저장 후 메인 메뉴로
    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("MainMenu")));
}

