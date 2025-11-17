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

	//Exec������(`)���� �Լ��� ȣ�� �Ҽ��ֵ���
	UFUNCTION(BlueprintCallable)
	void LoadMainMenu();

	UFUNCTION(BlueprintCallable)
	void LoadPauseMenu();

	//�����
	UFUNCTION(Exec)
	void Host(FString ServerName) override;
	UFUNCTION(Exec)
	void Join(uint32 Index) override;
	UFUNCTION(Exec)
	void RefreshServerList() override;
	void OpenMainMenuLevel() override;
	
	// 테스트용 콘솔 명령어 (UI 없이 테스트 가능)
	UFUNCTION(Exec)
	void TestHost(); // 테스트 서버 호스팅
	UFUNCTION(Exec)
	void TestJoin(); // 첫 번째 서버에 조인
	UFUNCTION(Exec)
	void TestNetworkStatus(); // 네트워크 상태 확인
	UFUNCTION(Exec)
	void TestRefresh(); // 서버 목록 새로고침 (RefreshServerList의 별칭)
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


	//���� ������ ��(����)�̸�
	FString DesiredServerName;
	IOnlineSessionPtr SessionInterface; //���� �����Ҷ� ���� �������̽�

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

};
