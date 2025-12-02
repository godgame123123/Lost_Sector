// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "OnlineSessionSettings.h"
#include "MainMenu.h"
#include "PauseMenu.h"
#include "UMultiplayerMenuWidget.h"
#include "UServerBrowserWidget.h"
#include "UCreateGameWidget.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"

//�ʺ�ä��,�߼�ä��
const static FName SESSION_NAME = TEXT("GameSession"); //ä�θ�
const static FName SESSION_SETTINGS_KEY = TEXT("FREE");//���Ӹ��

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
	
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] RefreshServerList: 세션 검색 시작 (최대 100개)"));
		//���� 100�� �ִ� ã�ƿ´�.
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")),true, EOnlineComparisonOp::Equals);
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
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] Join: 서버 조인 시도 - 인덱스: %d"), Index);
	
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] Join: SessionInterface가 유효하지 않습니다!"));
		return;
	}
	
	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] Join: SessionSearch가 유효하지 않습니다!"));
		return;
	}

	int32 NumResults = SessionSearch->SearchResults.Num();
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] Join: 검색된 서버 개수: %d"), NumResults);

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
		
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] Join: 서버 조인 시작 - 서버 이름: %s, 인덱스: %d"), 
			*ServerName, Index);
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] Join: 서버 정보 - 플레이어: %d/%d, 핑: %dms"), 
			SelectedResult.Session.NumOpenPublicConnections, 
			SelectedResult.Session.SessionSettings.NumPublicConnections,
			SelectedResult.PingInMs);
		
		SessionInterface->JoinSession(0, SESSION_NAME, SelectedResult);
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] Join: JoinSession 호출 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] Join: 잘못된 인덱스! 요청 인덱스: %d, 사용 가능한 서버 수: %d"), 
			Index, NumResults);
	}
}

void UMyGameInstance::OnCreateSessionComplate(FName InSessionName, bool IsSuccess)
{
	if (!IsSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not Createsession"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Session name is %s"), *InSessionName.ToString());

	UEngine* Engine = GetEngine();
	if (!Engine) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Host Complate!"));

	UWorld* World = GetWorld();
	if (!World) return;

	//����(��)
	// 로비 맵으로 이동 (Listen Server 시작)
	// DefaultEngine.ini의 GameModeMapPrefixes에 따라 LobbyGameMode가 자동 설정됨
	World->ServerTravel("RobbyMap?listen");
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
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnFindSessionComplate: 호출됨 - IsSuccess: %d"), IsSuccess ? 1 : 0);
	
	if (!IsSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] OnFindSessionComplate: 세션 검색 실패!"));
		return;
	}
	
	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] OnFindSessionComplate: SessionSearch가 유효하지 않습니다!"));
		return;
	}

	int32 NumFound = SessionSearch->SearchResults.Num();
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnFindSessionComplate: 찾은 서버 개수: %d"), NumFound);

	TArray<FServerData> ServerNames;

	for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
	{
		const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[i];
		
		FString ServerName;
		if (SearchResult.Session.SessionSettings.Get(SESSION_SETTINGS_KEY, ServerName))
		{
			UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnFindSessionComplate: [%d] 서버 이름: %s"), i, *ServerName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] OnFindSessionComplate: [%d] Session Name Not Found"), i);
			ServerName = FString::Printf(TEXT("Server_%d"), i);
		}
		
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnFindSessionComplate: [%d] 세션 ID: %s"), 
			i, *SearchResult.GetSessionIdStr());
		UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnFindSessionComplate: [%d] 플레이어: %d/%d, 핑: %dms, 호스트: %s"), 
			i,
			SearchResult.Session.NumOpenPublicConnections,
			SearchResult.Session.SessionSettings.NumPublicConnections,
			SearchResult.PingInMs,
			*SearchResult.Session.OwningUserName);

		FServerData ServerData;
		ServerData.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
		ServerData.CurrentPlayers = SearchResult.Session.NumOpenPublicConnections;
		ServerData.HostUserName = SearchResult.Session.OwningUserName;
		ServerData.Name = ServerName;

		ServerNames.Add(ServerData);
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

	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnFindSessionComplate: 서버 검색 완료 - 총 %d개 서버 발견"), NumFound);
}
void UMyGameInstance::OnJoinSessionComplate(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult)
{
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnJoinSessionComplate: 호출됨 - SessionName: %s, Result: %d"), 
		*InSessionName.ToString(), (int32)InResult);
	
	if (SessionInterface.IsValid() == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] OnJoinSessionComplate: SessionInterface가 유효하지 않습니다!"));
		return;
	}

	FString Address;//�ش� ���� �������ּ�
	if (!SessionInterface->GetResolvedConnectString(InSessionName, Address))
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] OnJoinSessionComplate: 서버 주소를 가져올 수 없습니다!"));
		
		UEngine* Engine = GetEngine();
	if (Engine)
	{
		FString ErrorMessage = TEXT("Failed to join session");
		switch (InResult)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ErrorMessage = TEXT("Session is full");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ErrorMessage = TEXT("Session does not exist");
			break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			ErrorMessage = TEXT("Could not retrieve server address");
			break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			ErrorMessage = TEXT("Already in session");
			break;
		default:
			ErrorMessage = TEXT("Unknown error occurred");
			break;
		}
			Engine->AddOnScreenDebugMessage(0, 5, FColor::Red, ErrorMessage);
			UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] OnJoinSessionComplate: 조인 실패 - %s"), *ErrorMessage);
		}
		LoadMainMenu();
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnJoinSessionComplate: 서버 주소 획득 성공 - %s"), *Address);
	
	UEngine* Engine = GetEngine();
	if (!Engine)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] OnJoinSessionComplate: Engine가 null입니다!"));
		return;
	}
	
	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining To %s"), *Address));
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnJoinSessionComplate: 서버로 이동 시작 - 주소: %s"), *Address);
	
	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] OnJoinSessionComplate: PlayerController가 null입니다!"));
		return;
	}
	
	PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	UE_LOG(LogTemp, Log, TEXT("[MyGameInstance] OnJoinSessionComplate: ClientTravel 호출 완료"));
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
