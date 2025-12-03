// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "OnlineSessionSettings.h"
#include "MainMenu.h"
#include "PauseMenu.h"
#include "UMultiplayerMenuWidget.h"
#include "UServerBrowserWidget.h"
#include "UCreateGameWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Framework/Application/SlateApplication.h"
#include "InventorySaveManager.h"
#include "ItemTypes.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"

//�ʺ�ä��,�߼�ä��
const static FName SESSION_NAME = TEXT("GameSession"); //ä�θ�
const static FName SESSION_SETTINGS_KEY = TEXT("FREE");//���Ӹ��
const static FName SESSION_HOST_NAME_KEY = TEXT("HOST_NAME"); // 호스트 플레이어 이름 (JSON의 playerName)
UMyGameInstance::UMyGameInstance()
{
	// 모든 위젯 클래스들은 런타임에 동적으로 로드하도록 변경 (패킹 에러 방지)
	// ConstructorHelpers::FClassFinder는 CDO 생성 시점에 에셋을 찾으려고 시도하므로
	// 패킹 과정에서 에셋이 없거나 경로가 잘못되면 실패합니다.
	// 따라서 모든 위젯 클래스는 각각의 Load...Menu() 함수에서 동적으로 로드됩니다.
}
void UMyGameInstance::LoadMainMenu()
{
	// MainMenuWidgetClass가 null이면 동적으로 로드 시도
	if (!MainMenuWidgetClass)
	{
		// 여러 경로 시도 (Blueprint 클래스는 _C 접미사 필요)
		static const TArray<FString> MainMenuPaths = {
			TEXT("/Game/Team_Folder/GimanLee/WB_MainMenu.WB_MainMenu_C"),
			TEXT("/Game/Team_Folder/LHJ/UI/WB_MainMenu.WB_MainMenu_C")
		};

		for (const FString& Path : MainMenuPaths)
		{
			if (UClass* FoundClass = LoadClass<UUserWidget>(nullptr, *Path))
			{
				MainMenuWidgetClass = FoundClass;
				UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] LoadMainMenu: WB_MainMenu 클래스 동적 로드 성공 (경로: %s)"), *Path);
				break;
			}
		}

		if (!MainMenuWidgetClass)
		{
			UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] LoadMainMenu: WB_MainMenu Blueprint를 찾을 수 없습니다. MainMenu 기능이 작동하지 않습니다."));
			return;
		}
	}

	MainMenu = CreateWidget<UMainMenu>(this, MainMenuWidgetClass);
	if (!MainMenu) 
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] LoadMainMenu: MainMenu 위젯 생성 실패!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] LoadMainMenu: MainMenu 위젯 생성 성공, SetOwningInstance 호출"));
	MainMenu->SetOwningInstance(TScriptInterface<IMyInterface>(this));
	MainMenu->StartUp();
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] LoadMainMenu: StartUp 완료"));
}

void UMyGameInstance::RegisterMainMenu(UMainMenu* InMainMenu)
{
	if (InMainMenu)
	{
		MainMenu = InMainMenu;
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] RegisterMainMenu: MainMenu 등록 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] RegisterMainMenu: InMainMenu가 null입니다!"));
	}
}

void UMyGameInstance::LoadPauseMenu()
{
	// PauseMenuWidgetClass가 null이면 동적으로 로드 시도
	if (!PauseMenuWidgetClass)
	{
		// 여러 경로 시도 (Blueprint 클래스는 _C 접미사 필요)
		static const TArray<FString> PauseMenuPaths = {
			TEXT("/Game/Team_Folder/GimanLee/WB_PauseMenu.WB_PauseMenu_C"),
			TEXT("/Game/UI/WB_PauseMenu.WB_PauseMenu_C"),
			TEXT("/Game/Team_Folder/LHJ/UI/WB_PauseMenu.WB_PauseMenu_C")
		};

		for (const FString& Path : PauseMenuPaths)
		{
			if (UClass* FoundClass = LoadClass<UUserWidget>(nullptr, *Path))
			{
				PauseMenuWidgetClass = FoundClass;
				UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] LoadPauseMenu: WB_PauseMenu 클래스 동적 로드 성공 (경로: %s)"), *Path);
				break;
			}
		}

		if (!PauseMenuWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] LoadPauseMenu: WB_PauseMenu Blueprint를 찾을 수 없습니다. PauseMenu 기능이 작동하지 않습니다."));
			return;
		}
	}

	PauseMenu = CreateWidget<UPauseMenu>(this, PauseMenuWidgetClass);
	if (!PauseMenu)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] LoadPauseMenu: PauseMenu 위젯 생성 실패!"));
		return;
	}

	PauseMenu->SetOwningInstance(TScriptInterface<IMyInterface>(this));
	PauseMenu->StartUp();
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] LoadPauseMenu: PauseMenu 위젯 생성 및 시작 완료"));
}

void UMyGameInstance::LoadMultiplayerMenu()
{
	// MultiplayerMenuWidgetClass가 null이면 동적으로 로드 시도
	if (!MultiplayerMenuWidgetClass)
	{
		static const TArray<FString> MultiplayerMenuPaths = {
			TEXT("/Game/Team_Folder/LHJ/UI/WB_MultiplayerMenu.WB_MultiplayerMenu_C")
		};

		for (const FString& Path : MultiplayerMenuPaths)
		{
			if (UClass* FoundClass = LoadClass<UUserWidget>(nullptr, *Path))
			{
				MultiplayerMenuWidgetClass = FoundClass;
				UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] LoadMultiplayerMenu: WB_MultiplayerMenu 클래스 동적 로드 성공"));
				break;
			}
		}

		if (!MultiplayerMenuWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] LoadMultiplayerMenu: WB_MultiplayerMenu Blueprint를 찾을 수 없습니다."));
			return;
		}
	}

	MultiplayerMenu = CreateWidget<UMultiplayerMenuWidget>(this, MultiplayerMenuWidgetClass);
	if (!MultiplayerMenu)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] LoadMultiplayerMenu: MultiplayerMenu 위젯 생성 실패!"));
		return;
	}

	MultiplayerMenu->SetOwningInstance(TScriptInterface<IMyInterface>(this));
	MultiplayerMenu->StartUp();
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] LoadMultiplayerMenu: MultiplayerMenu 위젯 생성 및 시작 완료"));
}

void UMyGameInstance::LoadServerBrowser()
{
	// ServerBrowserWidgetClass가 null이면 동적으로 로드 시도
	if (!ServerBrowserWidgetClass)
	{
		static const TArray<FString> ServerBrowserPaths = {
			TEXT("/Game/Team_Folder/LHJ/UI/WB_ServerBrowser.WB_ServerBrowser_C")
		};

		for (const FString& Path : ServerBrowserPaths)
		{
			if (UClass* FoundClass = LoadClass<UUserWidget>(nullptr, *Path))
			{
				ServerBrowserWidgetClass = FoundClass;
				UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] LoadServerBrowser: WB_ServerBrowser 클래스 동적 로드 성공"));
				break;
			}
		}

		if (!ServerBrowserWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] LoadServerBrowser: WB_ServerBrowser Blueprint를 찾을 수 없습니다."));
			return;
		}
	}

	ServerBrowser = CreateWidget<UServerBrowserWidget>(this, ServerBrowserWidgetClass);
	if (!ServerBrowser)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] LoadServerBrowser: ServerBrowser 위젯 생성 실패!"));
		return;
	}

	ServerBrowser->SetOwningGameInstance(this);
	ServerBrowser->AddToViewport(10);
	
	// Input mode 설정
	UWorld* World = GetWorld();
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ServerBrowser->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
	}
	
	RefreshServerList(); // Automatically refresh when opening server browser
}

void UMyGameInstance::LoadCreateGameMenu()
{
	// CreateGameWidgetClass가 null이면 동적으로 로드 시도
	if (!CreateGameWidgetClass)
	{
		static const TArray<FString> CreateGamePaths = {
			TEXT("/Game/Team_Folder/LHJ/UI/WB_CreateGame.WB_CreateGame_C")
		};

		for (const FString& Path : CreateGamePaths)
		{
			if (UClass* FoundClass = LoadClass<UUserWidget>(nullptr, *Path))
			{
				CreateGameWidgetClass = FoundClass;
				UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] LoadCreateGameMenu: WB_CreateGame 클래스 동적 로드 성공"));
				break;
			}
		}

		if (!CreateGameWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] LoadCreateGameMenu: WB_CreateGame Blueprint를 찾을 수 없습니다."));
			return;
		}
	}

	CreateGameMenu = CreateWidget<UUCreateGameWidget>(this, CreateGameWidgetClass);
	if (!CreateGameMenu)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] LoadCreateGameMenu: CreateGameMenu 위젯 생성 실패!"));
		return;
	}

	CreateGameMenu->SetOwningInstance(this);
	CreateGameMenu->AddToViewport(10);
	
	// Input mode 설정
	UWorld* World = GetWorld();
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(CreateGameMenu->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
	}
}


void UMyGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		UE_LOG(LogTemp, Warning, 
			TEXT("OSS : %s is Avaliable."), *OSS->GetSubsystemName().ToString());

		SessionInterface = OSS->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnCreateSessionComplate);

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnDestroySessionComplate);

			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnFindSessionComplate);

			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnJoinSessionComplate);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not found subsystem."));
	}

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UMyGameInstance::OnNetworkFailure);
	}

}



void UMyGameInstance::Host(FString ServerName, int32 MaxPlayers, bool bIsLan)
{
	DesiredServerName = ServerName;
	DesiredMaxPlayers = MaxPlayers;
	DesiredbIsLan = bIsLan;
	
	if (SessionInterface.IsValid())
	{
		auto AlreadyExsistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (AlreadyExsistingSession)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s is already exsist. re-createSession."),*SESSION_NAME.ToString());
			SessionInterface->DestroySession(SESSION_NAME);
			// DestroySession이 완료되면 OnDestroySessionComplate에서 CreateSession 호출됨
		}
		else
		{ 
			CreateSession(MaxPlayers, bIsLan);
		}
	}
}
void UMyGameInstance::CreateSession(int32 MaxPlayers, bool bIsLan)
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;

		// LAN 설정 (파라미터 우선, 없으면 NULL subsystem 체크)
		if (bIsLan || IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
			SessionSettings.bIsLANMatch = true;
		else
			SessionSettings.bIsLANMatch = false;


		// P2P 멀티플레이어를 위한 세션 설정
		SessionSettings.NumPublicConnections = MaxPlayers;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bAllowInvites = true; // P2P 초대 허용
		SessionSettings.bAllowJoinInProgress = true; // 게임 중 참가 허용
		
		SessionSettings.Set(
			SESSION_SETTINGS_KEY, DesiredServerName,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		
		// 호스트 플레이어 이름을 JSON에서 로드하여 세션 설정에 추가
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		UE_LOG(LogTemp, Warning, TEXT("[서버 생성] 호스트 플레이어 이름 로드 시작"));
		FString HostPlayerName = TEXT("Unknown Host");
		FString PlayerID;
		
		// 방법 1: PlayerState가 있으면 사용
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				UE_LOG(LogTemp, Log, TEXT("[서버 생성] PlayerController 찾음: %s"), *GetNameSafe(PC));
				if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
				{
					UE_LOG(LogTemp, Log, TEXT("[서버 생성] PlayerState 찾음: %s"), *GetNameSafe(PS));
					if (PS->GetUniqueId().IsValid())
					{
						PlayerID = PS->GetUniqueId()->ToString();
						UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ✅ PlayerState에서 PlayerID 획득: %s"), *PlayerID);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ⚠️ PlayerState의 UniqueId가 유효하지 않음"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ⚠️ PlayerState가 아직 없음 (PlayerController는 있음)"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ⚠️ PlayerController가 없음"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ⚠️ World가 없음"));
		}
		
		// 방법 2: PlayerState가 없으면 Online Subsystem의 Identity Interface 사용
		if (PlayerID.IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("[서버 생성] PlayerID가 비어있어 Online Subsystem 시도"));
			IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
			if (OSS)
			{
				UE_LOG(LogTemp, Log, TEXT("[서버 생성] Online Subsystem 찾음: %s"), *OSS->GetSubsystemName().ToString());
				IOnlineIdentityPtr IdentityInterface = OSS->GetIdentityInterface();
				if (IdentityInterface.IsValid())
				{
					// 로컬 플레이어의 고유 ID 가져오기 (일반적으로 인덱스 0)
					// GetUniquePlayerId는 FUniqueNetIdPtr (const)를 반환하므로 const 타입으로 받아야 함
					FUniqueNetIdPtr UniqueNetId = IdentityInterface->GetUniquePlayerId(0);
					if (UniqueNetId.IsValid())
					{
						PlayerID = UniqueNetId->ToString();
						UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ✅ Online Subsystem에서 PlayerID 획득: %s"), *PlayerID);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ⚠️ IdentityInterface에서 UniqueNetId를 가져올 수 없음"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ⚠️ IdentityInterface가 유효하지 않음"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ⚠️ Online Subsystem이 없음"));
			}
		}
		
		// PlayerID를 찾았으면 JSON에서 플레이어 이름 로드
		if (!PlayerID.IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("[서버 생성] JSON 파일 로드 시도 (PlayerID: %s)"), *PlayerID);
			FPlayerInventorySaveData PlayerData;
			if (UInventorySaveManager::LoadPlayerInventory(this, PlayerID, PlayerData))
			{
				UE_LOG(LogTemp, Log, TEXT("[서버 생성] JSON 파일 로드 성공"));
				if (!PlayerData.PlayerName.IsEmpty())
				{
					HostPlayerName = PlayerData.PlayerName;
					UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ✅ JSON에서 플레이어 이름 로드 성공: %s (ID: %s)"), *HostPlayerName, *PlayerID);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ⚠️ JSON에 플레이어 이름이 없습니다 (ID: %s)"), *PlayerID);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ❌ JSON 파일을 찾을 수 없습니다 (ID: %s)"), *PlayerID);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[서버 생성] ❌ PlayerID를 획득할 수 없어 기본값 사용"));
		}
		
		SessionSettings.Set(
			SESSION_HOST_NAME_KEY, HostPlayerName,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] CreateSession: 호스트 플레이어 이름 설정 - %s"), *HostPlayerName);


		if (MainMenu)
			MainMenu->Shutdown();

		//�����
		SessionInterface->CreateSession(0,SESSION_NAME,SessionSettings);
	}
}

void UMyGameInstance::RefreshServerList()
{
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] RefreshServerList: 서버 목록 새로고침 시작"));
	
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] RefreshServerList: SessionInterface가 유효하지 않습니다!"));
		return;
	}
	
	// 이미 검색 중이면 무시
	if (bIsSearchingForSessions)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] RefreshServerList: 이미 검색 중입니다. 요청을 무시합니다."));
		return;
	}
	
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] RefreshServerList: 세션 검색 시작 (최대 100개)"));
		//���� 100�� �ִ� ã�ƿ´�.
		bIsSearchingForSessions = true;
		
		// NULL 서브시스템이면 LAN 검색
		IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
		bool bIsLan = (OSS && OSS->GetSubsystemName() == "NULL");
		
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->bIsLanQuery = bIsLan;
		
		// Presence 검색 설정
		SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);
		
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] RefreshServerList: 검색 모드 - LAN: %d, OSS: %s"), 
			bIsLan ? 1 : 0, OSS ? *OSS->GetSubsystemName().ToString() : TEXT("None"));
		
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] RefreshServerList: FindSessions 호출 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] RefreshServerList: SessionSearch 생성 실패!"));
	}
}
void UMyGameInstance::OpenMainMenuLevel()
{
	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC) return;

	PC->ClientTravel("/Game/Maps/Main",ETravelType::TRAVEL_Absolute);
}
void UMyGameInstance::Join(uint32 Index)
{
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	UE_LOG(LogTemp, Warning, TEXT("[서버 조인] 서버 조인 시도 시작"));
	UE_LOG(LogTemp, Warning, TEXT("  - 선택된 서버 인덱스: %d"), Index);
	
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 조인] ❌ SessionInterface가 유효하지 않습니다!"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}
	
	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 조인] ❌ SessionSearch가 유효하지 않습니다!"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}

	int32 NumResults = SessionSearch->SearchResults.Num();
	UE_LOG(LogTemp, Warning, TEXT("[서버 조인] 검색된 서버 개수: %d"), NumResults);

	if (MainMenu)
	{
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] Join: MainMenu 종료"));
		MainMenu->Shutdown();
	}

	if(SessionSearch->SearchResults.Num() > (int32)Index)
	{
		const FOnlineSessionSearchResult& SelectedResult = SessionSearch->SearchResults[Index];
		FString ServerName;
		SelectedResult.Session.SessionSettings.Get(SESSION_SETTINGS_KEY, ServerName);
		
		FString HostName;
		SelectedResult.Session.SessionSettings.Get(SESSION_HOST_NAME_KEY, HostName);
		if (HostName.IsEmpty())
		{
			HostName = SelectedResult.Session.OwningUserName;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[서버 조인] 📋 선택된 서버 정보:"));
		UE_LOG(LogTemp, Warning, TEXT("  - 서버 이름: %s"), *ServerName);
		UE_LOG(LogTemp, Warning, TEXT("  - 호스트 이름: %s"), *HostName);
		UE_LOG(LogTemp, Warning, TEXT("  - 현재 플레이어: %d/%d"), 
			SelectedResult.Session.SessionSettings.NumPublicConnections - SelectedResult.Session.NumOpenPublicConnections,
			SelectedResult.Session.SessionSettings.NumPublicConnections);
		UE_LOG(LogTemp, Warning, TEXT("  - 빈 슬롯: %d"), SelectedResult.Session.NumOpenPublicConnections);
		UE_LOG(LogTemp, Warning, TEXT("  - 핑: %dms"), SelectedResult.PingInMs);
		
		UE_LOG(LogTemp, Warning, TEXT("[서버 조인] 🔄 JoinSession 호출 중..."));
		SessionInterface->JoinSession(0, SESSION_NAME, SelectedResult);
		UE_LOG(LogTemp, Warning, TEXT("[서버 조인] ✅ JoinSession 호출 완료 - 세션 조인 대기 중..."));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 조인] ❌ 잘못된 인덱스!"));
		UE_LOG(LogTemp, Error, TEXT("  - 요청 인덱스: %d"), Index);
		UE_LOG(LogTemp, Error, TEXT("  - 사용 가능한 서버 수: %d"), NumResults);
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	}
}

void UMyGameInstance::OnCreateSessionComplate(FName InSessionName, bool IsSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	UE_LOG(LogTemp, Warning, TEXT("[서버 생성 완료] OnCreateSessionComplate 호출됨"));
	UE_LOG(LogTemp, Warning, TEXT("  - SessionName: %s"), *InSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("  - IsSuccess: %d"), IsSuccess ? 1 : 0);
	
	if (!IsSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 생성 완료] ❌ 세션 생성 실패!"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}

	// 세션이 제대로 생성되었는지 확인
	if (SessionInterface.IsValid())
	{
		auto CreatedSession = SessionInterface->GetNamedSession(InSessionName);
		if (CreatedSession)
		{
			UE_LOG(LogTemp, Warning, TEXT("[서버 생성 완료] ✅ 세션 생성 확인됨"));
			UE_LOG(LogTemp, Warning, TEXT("  - 세션 ID: %s"), *CreatedSession->GetSessionIdStr());
			UE_LOG(LogTemp, Warning, TEXT("  - 최대 플레이어: %d"), CreatedSession->SessionSettings.NumPublicConnections);
			UE_LOG(LogTemp, Warning, TEXT("  - 빈 슬롯: %d"), CreatedSession->NumOpenPublicConnections);
			UE_LOG(LogTemp, Warning, TEXT("  - LAN 매치: %d"), CreatedSession->SessionSettings.bIsLANMatch ? 1 : 0);
			UE_LOG(LogTemp, Warning, TEXT("  - Presence 사용: %d"), CreatedSession->SessionSettings.bUsesPresence ? 1 : 0);
			UE_LOG(LogTemp, Warning, TEXT("  - 광고 허용: %d"), CreatedSession->SessionSettings.bShouldAdvertise ? 1 : 0);
			
			FString ServerName;
			if (CreatedSession->SessionSettings.Get(SESSION_SETTINGS_KEY, ServerName))
			{
				UE_LOG(LogTemp, Warning, TEXT("  - 서버 이름: %s"), *ServerName);
			}
			
			FString HostName;
			if (CreatedSession->SessionSettings.Get(SESSION_HOST_NAME_KEY, HostName))
			{
				UE_LOG(LogTemp, Warning, TEXT("  - 호스트 이름: %s"), *HostName);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[서버 생성 완료] ⚠️ 세션이 생성되었지만 GetNamedSession에서 찾을 수 없습니다!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 생성 완료] ⚠️ SessionInterface가 유효하지 않습니다!"));
	}

	UE_LOG(LogTemp, Warning, TEXT("Session name is %s"), *InSessionName.ToString());

	UEngine* Engine = GetEngine();
	if (!Engine)
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 생성 완료] ❌ Engine가 null입니다!"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}

	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, TEXT("Host Complete!"));
	UE_LOG(LogTemp, Warning, TEXT("[서버 생성 완료] 🚀 로비 맵으로 이동 시작"));

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 생성 완료] ❌ World가 null입니다!"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}

	//����(��)
	// 로비 맵으로 이동 (Listen Server 시작)
	// DefaultEngine.ini의 GameModeMapPrefixes에 따라 LobbyGameMode가 자동 설정됨
	UE_LOG(LogTemp, Warning, TEXT("[서버 생성 완료] ServerTravel 호출: RobbyMap?listen"));
	World->ServerTravel("RobbyMap?listen");
	UE_LOG(LogTemp, Warning, TEXT("[서버 생성 완료] ✅ ServerTravel 호출 완료"));
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	//World->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
}


void UMyGameInstance::StartSession()
{
	if (SessionInterface.IsValid())
		SessionInterface->StartSession(SESSION_NAME);
}
void UMyGameInstance::OnDestroySessionComplate(FName InSessionName, bool IsSuccess)
{
	if (IsSuccess == true)
		CreateSession(DesiredMaxPlayers, DesiredbIsLan);
}

void UMyGameInstance::OnFindSessionComplate(bool IsSuccess)
{
	// 검색 완료 플래그 리셋
	bIsSearchingForSessions = false;
	
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	UE_LOG(LogTemp, Warning, TEXT("[서버 검색 완료] OnFindSessionComplate 호출됨"));
	UE_LOG(LogTemp, Warning, TEXT("  - IsSuccess: %d"), IsSuccess ? 1 : 0);
	
	if (!IsSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 검색 완료] ❌ 세션 검색 실패!"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}
	
	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 검색 완료] ❌ SessionSearch가 유효하지 않습니다!"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}
	
	int32 NumFound = SessionSearch->SearchResults.Num();
	UE_LOG(LogTemp, Warning, TEXT("[서버 검색 완료] 찾은 서버 개수: %d"), NumFound);
	
	// 현재 호스트인지 확인 (자신이 호스트인 경우 자신의 세션이 검색 결과에 포함되는지 확인)
	if (SessionInterface.IsValid())
	{
		auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (ExistingSession)
		{
			UE_LOG(LogTemp, Warning, TEXT("[서버 검색 완료] ⚠️ 현재 호스트입니다! 자신의 세션이 검색 결과에 포함되는지 확인합니다."));
			FString MySessionId = ExistingSession->GetSessionIdStr();
			UE_LOG(LogTemp, Warning, TEXT("  - 내 세션 ID: %s"), *MySessionId);
			
			bool bFoundMySession = false;
			for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
			{
				FString FoundSessionId = SessionSearch->SearchResults[i].GetSessionIdStr();
				if (FoundSessionId == MySessionId)
				{
					bFoundMySession = true;
					UE_LOG(LogTemp, Warning, TEXT("  - ✅ 자신의 세션이 검색 결과 [%d]에 포함되어 있습니다!"), i);
					break;
				}
			}
			
			if (!bFoundMySession)
			{
				UE_LOG(LogTemp, Warning, TEXT("  - ⚠️ 자신의 세션이 검색 결과에 포함되지 않았습니다 (NULL Online Subsystem에서는 정상일 수 있음)"));
			}
		}
	}

	TArray<FServerData> ServerNames;

	for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
	{
		const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[i];
		
		UE_LOG(LogTemp, Warning, TEXT("[서버 검색 완료] ─────────────────────────────────────────────"));
		UE_LOG(LogTemp, Warning, TEXT("[서버 검색 완료] 서버 [%d] 정보:"), i);
		
		FString ServerName;
		if (SearchResult.Session.SessionSettings.Get(SESSION_SETTINGS_KEY, ServerName))
		{
			UE_LOG(LogTemp, Warning, TEXT("  - 서버 이름: %s"), *ServerName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("  - 서버 이름: Session Name Not Found (기본값 사용)"));
			ServerName = FString::Printf(TEXT("Server_%d"), i);
		}
		
		FString SessionId = SearchResult.GetSessionIdStr();
		UE_LOG(LogTemp, Warning, TEXT("  - 세션 ID: %s"), *SessionId);
		UE_LOG(LogTemp, Warning, TEXT("  - 플레이어: %d/%d"), 
			SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections,
			SearchResult.Session.SessionSettings.NumPublicConnections);
		UE_LOG(LogTemp, Warning, TEXT("  - 빈 슬롯: %d"), SearchResult.Session.NumOpenPublicConnections);
		UE_LOG(LogTemp, Warning, TEXT("  - 핑: %dms"), SearchResult.PingInMs);
		UE_LOG(LogTemp, Warning, TEXT("  - 호스트 사용자명: %s"), *SearchResult.Session.OwningUserName);
		UE_LOG(LogTemp, Warning, TEXT("  - LAN 매치: %d"), SearchResult.Session.SessionSettings.bIsLANMatch ? 1 : 0);

		FServerData ServerData;
		ServerData.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
		ServerData.CurrentPlayers = SearchResult.Session.NumOpenPublicConnections;
		
		// 호스트 이름 설정: 세션 설정에서 먼저 시도 (JSON의 playerName), 없으면 OwningUserName 사용
		FString HostName;
		if (!SearchResult.Session.SessionSettings.Get(SESSION_HOST_NAME_KEY, HostName) || HostName.IsEmpty())
		{
			// 세션 설정에 없으면 OwningUserName 사용
			HostName = SearchResult.Session.OwningUserName;
			if (HostName.IsEmpty())
			{
				HostName = TEXT("Unknown Host");
			}
			UE_LOG(LogTemp, Warning, TEXT("  - 호스트 이름: %s (OwningUserName에서 로드)"), *HostName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("  - 호스트 이름: %s (JSON playerName에서 로드)"), *HostName);
		}
		ServerData.HostUserName = HostName;
		ServerData.Name = ServerName;

		ServerNames.Add(ServerData);
		UE_LOG(LogTemp, Warning, TEXT("[서버 검색 완료] ─────────────────────────────────────────────"));
	}

	// 기존 MainMenu에 서버 목록 전달
	if (MainMenu)
	{
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnFindSessionComplate: MainMenu에 서버 목록 전달 (%d개)"), ServerNames.Num());
		MainMenu->SetServerList(ServerNames);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] OnFindSessionComplate: MainMenu가 null입니다! 서버 목록을 전달할 수 없습니다."));
		UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] OnFindSessionComplate: LoadMainMenu()가 호출되었는지 확인하세요."));
 	}

	// 새로운 MultiplayerMenu의 ServerBrowser에도 서버 목록 전달
	if (MultiplayerMenu)
	{
		if (UServerBrowserWidget* BrowserWidget = MultiplayerMenu->GetServerBrowserWidget())
		{
			UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnFindSessionComplate: MultiplayerMenu의 ServerBrowser에 서버 목록 전달 (%d개)"), ServerNames.Num());
			BrowserWidget->SetServerList(ServerNames);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] OnFindSessionComplate: MultiplayerMenu의 ServerBrowserWidget를 찾을 수 없습니다!"));
		}
	}

	// LoadServerBrowser()로 직접 생성된 ServerBrowser에도 서버 목록 전달
	if (ServerBrowser)
	{
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnFindSessionComplate: ServerBrowser에 서버 목록 전달 (%d개)"), ServerNames.Num());
		ServerBrowser->SetServerList(ServerNames);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] OnFindSessionComplate: ServerBrowser가 null입니다! LoadServerBrowser()가 호출되었는지 확인하세요."));
	}

	UE_LOG(LogTemp, Warning, TEXT("[서버 검색 완료] ✅ 서버 검색 완료 - 총 %d개 서버 발견"), NumFound);
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
}
void UMyGameInstance::OnJoinSessionComplate(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult)
{
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	UE_LOG(LogTemp, Warning, TEXT("[서버 조인] OnJoinSessionComplate 호출됨"));
	UE_LOG(LogTemp, Warning, TEXT("  - SessionName: %s"), *InSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("  - Result Code: %d"), (int32)InResult);
	
	if (SessionInterface.IsValid() == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 조인] ❌ SessionInterface가 유효하지 않습니다!"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}

	FString Address;//�ش� ���� �������ּ�
	if (!SessionInterface->GetResolvedConnectString(InSessionName, Address))
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 조인] ❌ 서버 주소를 가져올 수 없습니다!"));
		
		UEngine* Engine = GetEngine();
		if (Engine)
		{
			FString ErrorMessage = TEXT("Failed to join session");
			switch (InResult)
			{
			case EOnJoinSessionCompleteResult::SessionIsFull:
				ErrorMessage = TEXT("Session is full");
				UE_LOG(LogTemp, Error, TEXT("[서버 조인] ❌ 서버가 가득 찼습니다!"));
				break;
			case EOnJoinSessionCompleteResult::SessionDoesNotExist:
				ErrorMessage = TEXT("Session does not exist");
				UE_LOG(LogTemp, Error, TEXT("[서버 조인] ❌ 서버가 존재하지 않습니다!"));
				break;
			case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
				ErrorMessage = TEXT("Could not retrieve server address");
				UE_LOG(LogTemp, Error, TEXT("[서버 조인] ❌ 서버 주소를 가져올 수 없습니다!"));
				break;
			case EOnJoinSessionCompleteResult::AlreadyInSession:
				ErrorMessage = TEXT("Already in session");
				UE_LOG(LogTemp, Warning, TEXT("[서버 조인] ⚠️ 이미 세션에 참가 중입니다!"));
				break;
			default:
				ErrorMessage = TEXT("Unknown error occurred");
				UE_LOG(LogTemp, Error, TEXT("[서버 조인] ❌ 알 수 없는 오류 발생!"));
				break;
			}
			Engine->AddOnScreenDebugMessage(0, 5, FColor::Red, ErrorMessage);
			UE_LOG(LogTemp, Error, TEXT("[서버 조인] 조인 실패 - %s"), *ErrorMessage);
		}
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		LoadMainMenu();
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[서버 조인] GetResolvedConnectString 성공 - 원본 주소: %s"), *Address);
	
	// NULL Online Subsystem에서는 포트 번호가 0으로 나올 수 있으므로 수정
	// 주소 형식: "IP:PORT" 또는 "IP:0"
	FString FinalAddress = Address;
	int32 ColonIndex;
	if (Address.FindChar(TEXT(':'), ColonIndex))
	{
		FString IPPart = Address.Left(ColonIndex);
		FString PortPart = Address.Mid(ColonIndex + 1);
		
		UE_LOG(LogTemp, Warning, TEXT("[서버 조인] 주소 파싱 - IP: %s, Port: %s"), *IPPart, *PortPart);
		
		// 포트가 0이거나 비어있으면 기본 포트(7777) 사용
		int32 Port = FCString::Atoi(*PortPart);
		if (PortPart.IsEmpty() || Port == 0)
		{
			FinalAddress = FString::Printf(TEXT("%s:7777"), *IPPart);
			UE_LOG(LogTemp, Warning, TEXT("[서버 조인] ⚠️ 포트 번호가 0이므로 기본 포트(7777)로 변경"));
			UE_LOG(LogTemp, Warning, TEXT("  - 변경 전: %s"), *Address);
			UE_LOG(LogTemp, Warning, TEXT("  - 변경 후: %s"), *FinalAddress);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[서버 조인] 포트 번호 정상: %d"), Port);
		}
	}
	else
	{
		// 포트가 없으면 기본 포트 추가
		FinalAddress = FString::Printf(TEXT("%s:7777"), *Address);
		UE_LOG(LogTemp, Warning, TEXT("[서버 조인] ⚠️ 포트 번호가 없으므로 기본 포트(7777) 추가"));
		UE_LOG(LogTemp, Warning, TEXT("  - 변경 전: %s"), *Address);
		UE_LOG(LogTemp, Warning, TEXT("  - 변경 후: %s"), *FinalAddress);
	}
	
	Address = FinalAddress; // 최종 주소로 교체
	UE_LOG(LogTemp, Warning, TEXT("[서버 조인] ✅ 최종 서버 주소: %s"), *Address);
	
	UEngine* Engine = GetEngine();
	if (!Engine)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] OnJoinSessionComplate: Engine가 null입니다!"));
		return;
	}
	
	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining To %s"), *Address));
	UE_LOG(LogTemp, Warning, TEXT("[서버 조인] 🚀 서버로 이동 시작"));
	UE_LOG(LogTemp, Warning, TEXT("  - 목적지 주소: %s"), *Address);
	
	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[서버 조인] ❌ PlayerController가 null입니다!"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}
	
	// ClientTravel은 IP:PORT 형식만 받습니다 (맵 경로는 자동으로 처리됨)
	// 하지만 포트가 제대로 전달되도록 주소 형식을 확인합니다
	FString TravelAddress = Address;
	
	// 주소에 포트가 포함되어 있는지 확인
	if (!TravelAddress.Contains(TEXT(":")))
	{
		// 포트가 없으면 기본 포트 추가
		TravelAddress = FString::Printf(TEXT("%s:7777"), *TravelAddress);
		UE_LOG(LogTemp, Warning, TEXT("[서버 조인] ⚠️ 주소에 포트가 없어 기본 포트 추가: %s"), *TravelAddress);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[서버 조인] ClientTravel 호출 중..."));
	UE_LOG(LogTemp, Warning, TEXT("  - TravelAddress: %s"), *TravelAddress);
	UE_LOG(LogTemp, Warning, TEXT("  - TravelType: TRAVEL_Absolute"));
	
	PC->ClientTravel(TravelAddress, ETravelType::TRAVEL_Absolute);
	
	UE_LOG(LogTemp, Warning, TEXT("[서버 조인] ✅ ClientTravel 호출 완료 - 서버 연결 시도 중"));
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
}

void UMyGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	// 네트워크 실패 시 처리
	UE_LOG(LogTemp, Error, TEXT("Network Failure: %s (Type: %d)"), *ErrorString, (int32)FailureType);
	
	UEngine* Engine = GetEngine();
	if (Engine)
	{
		FString ErrorMessage = FString::Printf(TEXT("Network Error: %s"), *ErrorString);
		Engine->AddOnScreenDebugMessage(0, 5, FColor::Red, ErrorMessage);
	}

	// 클라이언트인 경우에만 메인 메뉴로 돌아가기 (P2P는 Listen Server 사용)
	if (World && World->GetNetMode() == NM_Client)
	{
		// 세션 정리
		if (SessionInterface.IsValid())
		{
			SessionInterface->DestroySession(SESSION_NAME);
		}
		
		// 메인 메뉴로 돌아가기
		OpenMainMenuLevel();
	}
}

// 테스트용 콘솔 명령어
void UMyGameInstance::TestHost()
{
	UE_LOG(LogTemp, Warning, TEXT("TestHost: Creating test server..."));
	Host(TEXT("Test Server"));
}

void UMyGameInstance::TestJoin()
{
	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TestJoin: No server list. Refreshing..."));
		RefreshServerList();
		return;
	}
	
	if (SessionSearch->SearchResults.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestJoin: Joining first server in list..."));
		Join(0);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TestJoin: No servers found. Use RefreshServerList first."));
	}
}

void UMyGameInstance::TestNetworkStatus()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestNetworkStatus: No World"));
		return;
	}
	
	ENetMode NetMode = World->GetNetMode();
	FString NetModeStr;
	switch (NetMode)
	{
	case NM_Standalone:
		NetModeStr = TEXT("Standalone");
		break;
	case NM_DedicatedServer:
		NetModeStr = TEXT("Dedicated Server");
		break;
	case NM_ListenServer:
		NetModeStr = TEXT("Listen Server");
		break;
	case NM_Client:
		NetModeStr = TEXT("Client");
		break;
	default:
		NetModeStr = TEXT("Unknown");
		break;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("=== Network Status ==="));
	UE_LOG(LogTemp, Warning, TEXT("NetMode: %s"), *NetModeStr);
	UE_LOG(LogTemp, Warning, TEXT("NumPlayers: %d"), World->GetNumPlayerControllers());
	
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		UE_LOG(LogTemp, Warning, TEXT("Online Subsystem: %s"), *OSS->GetSubsystemName().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Online Subsystem: Not found"));
	}
	
	if (SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Interface: Valid"));
		auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (ExistingSession)
		{
			UE_LOG(LogTemp, Warning, TEXT("Active Session: Yes"));
			UE_LOG(LogTemp, Warning, TEXT("  - Open Connections: %d"), ExistingSession->NumOpenPublicConnections);
			UE_LOG(LogTemp, Warning, TEXT("  - Max Connections: %d"), ExistingSession->SessionSettings.NumPublicConnections);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Active Session: No"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Interface: Invalid"));
	}
	UE_LOG(LogTemp, Warning, TEXT("======================"));
}

void UMyGameInstance::TestRefresh()
{
	UE_LOG(LogTemp, Warning, TEXT("TestRefresh: Refreshing server list..."));
	RefreshServerList();
}

// UI 테스트용 콘솔 명령어
void UMyGameInstance::TestOpenMultiplayerMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("TestOpenMultiplayerMenu: Opening multiplayer menu..."));
	LoadMultiplayerMenu();
}

void UMyGameInstance::TestOpenServerBrowser()
{
	UE_LOG(LogTemp, Warning, TEXT("TestOpenServerBrowser: Opening server browser..."));
	LoadServerBrowser();
}

void UMyGameInstance::TestOpenCreateGame()
{
	UE_LOG(LogTemp, Warning, TEXT("TestOpenCreateGame: Opening create game menu..."));
	LoadCreateGameMenu();
}
