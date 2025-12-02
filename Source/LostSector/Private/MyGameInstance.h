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
class UMultiplayerMenuWidget;
class UServerBrowserWidget;
USTRUCT(BlueprintType)
struct FServerData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	FString Name;
	
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentPlayers = 0;
	
	UPROPERTY(BlueprintReadWrite)
	int32 MaxPlayers = 0;
	
	UPROPERTY(BlueprintReadWrite)
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

	// 새로운 멀티플레이어 메뉴 로드
	UFUNCTION(BlueprintCallable)
	void LoadMultiplayerMenu();
	
	UFUNCTION(BlueprintCallable)
	void LoadServerBrowser();
	
	UFUNCTION(BlueprintCallable)
	void LoadCreateGameMenu();

	//�����
	UFUNCTION(Exec)
	void Host(FString ServerName, int32 MaxPlayers = 3, bool bIsLan = false) override;
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
	
	// UI 테스트용 콘솔 명령어
	UFUNCTION(Exec)
	void TestOpenMultiplayerMenu(); // 멀티플레이어 메뉴 열기
	UFUNCTION(Exec)
	void TestOpenServerBrowser(); // 서버 브라우저 열기
	UFUNCTION(Exec)
	void TestOpenCreateGame(); // 게임 생성 메뉴 열기
private:
	void OnCreateSessionComplate(FName InSessionName, bool IsSuccess);
	void OnDestroySessionComplate(FName InSessionName, bool IsSuccess);
	void OnFindSessionComplate(bool IsSuccess);
	void OnJoinSessionComplate(FName InSessionName,EOnJoinSessionCompleteResult::Type InResult);
	void OnNetworkFailure(UWorld* World,UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	void CreateSession(int32 MaxPlayers = 3, bool bIsLan = false);


public:
	UFUNCTION(BlueprintCallable)
	void StartSession();
private:


	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	class UMainMenu* MainMenu;

	TSubclassOf<UUserWidget> PauseMenuWidgetClass;
	class UPauseMenu* PauseMenu;

	TSubclassOf<UUserWidget> MultiplayerMenuWidgetClass;
	class UMultiplayerMenuWidget* MultiplayerMenu;

	TSubclassOf<UUserWidget> ServerBrowserWidgetClass;
	class UServerBrowserWidget* ServerBrowser;

	TSubclassOf<UUserWidget> CreateGameWidgetClass;
	class UUCreateGameWidget* CreateGameMenu;


	//���� ������ ��(����)�̸�
	FString DesiredServerName;
	int32 DesiredMaxPlayers = 3;
	bool DesiredbIsLan = false;
	IOnlineSessionPtr SessionInterface; //���� �����Ҷ� ���� �������̽�

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

};
