// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MyInterface.h"

#include "MyGameInstance.generated.h"


class UMainMenu;
class UPauseMenu;
USTRUCT()
struct FServerData
{
	GENERATED_BODY()
public:
	FString Name;
	uint16 CurrentPlayers;
	uint16 MaxPlayers;
	FString HostUserName;
};

UCLASS()
class UMyGameInstance : public UGameInstance, public IMyInterface
{
	GENERATED_BODY()
public:
	UMyGameInstance();

protected:
	virtual void Init() override;
public:

	//Exec명령줄(`)에서 함수를 호출 할수있도록
	UFUNCTION(BlueprintCallable)
	void LoadMainMenu();

	UFUNCTION(BlueprintCallable)
	void LoadPauseMenu();

	//방생성
	UFUNCTION(Exec)
	void Host(FString ServerName) override;
	UFUNCTION(Exec)
	void Join(uint32 Index) override;
	UFUNCTION(Exec)
	void RefreshServerList() override;
	void OpenMainMenuLevel() override;
private:
	void OnCreateSessionComplate(FName InSessionName, bool IsSuccess);
	void OnDestroySessionComplate(FName InSessionName, bool IsSuccess);
	void OnFindSessionComplate(bool IsSuccess);
	void OnJoinSessionComplate(FName InSessionName,EOnJoinSessionCompleteResult::Type InResult);
	void OnNetworkFailure(UWorld* World,UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	void CreateSession();


public:
	UFUNCTION(BlueprintCallable)
	void StartSession();
private:


	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	class UMainMenu* MainMenu;

	TSubclassOf<UUserWidget> PauseMenuWidgetClass;
	class UPauseMenu* PauseMenu;


	//내가 생성한 방(세션)이름
	FString DesiredServerName;
	IOnlineSessionPtr SessionInterface; //세션 생성할때 쓰는 인터페이스

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

};
