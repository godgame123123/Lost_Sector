// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "OnlineSessionSettings.h"
#include "MainMenu.h"
#include "PauseMenu.h"

//�ʺ�ä��,�߼�ä��
const static FName SESSION_NAME = TEXT("GameSession"); //ä�θ�
const static FName SESSION_SETTINGS_KEY = TEXT("FREE");//���Ӹ��

UMyGameInstance::UMyGameInstance()
{
	ConstructorHelpers::FClassFinder<UUserWidget> MainMenuBPClass(TEXT("/Game/UI/WB_MainMenu"));
	if (MainMenuBPClass.Succeeded())
		MainMenuWidgetClass = MainMenuBPClass.Class;

	ConstructorHelpers::FClassFinder<UUserWidget> PauseMenuBPClass(TEXT("/Game/UI/WB_PauseMenu"));
	if (PauseMenuBPClass.Succeeded())
		PauseMenuWidgetClass = PauseMenuBPClass.Class;
}
void UMyGameInstance::LoadMainMenu()
{
	if (!ensure(MainMenuWidgetClass)) return;

	MainMenu =  CreateWidget<UMainMenu>(this, MainMenuWidgetClass);
	if (!MainMenu) return;

	MainMenu->SetOwningInstance(this);
	MainMenu->StartUp();
}

void UMyGameInstance::LoadPauseMenu()
{
	if (!ensure(PauseMenuWidgetClass)) return;

	PauseMenu = CreateWidget<UPauseMenu>(this, PauseMenuWidgetClass);
	if (!PauseMenu) return;

	PauseMenu->SetOwningInstance(this);
	PauseMenu->StartUp();
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



void UMyGameInstance::Host(FString ServerName)
{
	DesiredServerName = ServerName;
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
			CreateSession();
		}
	}
}
void UMyGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;

		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
			SessionSettings.bIsLANMatch = true;
		else
			SessionSettings.bIsLANMatch = false;


		// P2P 멀티플레이어를 위한 세션 설정
		SessionSettings.NumPublicConnections = 24;
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
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Finding Session"));
		//���� 100�� �ִ� ã�ƿ´�.
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")),true, EOnlineComparisonOp::Equals);
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
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
	if (!SessionInterface.IsValid()) return;
	if (!SessionSearch.IsValid()) return;

	if (MainMenu)
		MainMenu->Shutdown();

	if(SessionSearch->SearchResults.Num() > (int32)Index)
		SessionInterface->JoinSession(0,SESSION_NAME,SessionSearch->SearchResults[Index]);
	else
		UE_LOG(LogTemp, Warning, TEXT("Empty Session"));
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
		CreateSession();
}

void UMyGameInstance::OnFindSessionComplate(bool IsSuccess)
{
	if (IsSuccess && SessionSearch.IsValid())
	{
		TArray<FServerData> ServerNames;

		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Display, TEXT("Found Session name : %s"), *SearchResult.GetSessionIdStr());
			UE_LOG(LogTemp, Display, TEXT("Ping : %d"), SearchResult.PingInMs);

			FServerData ServerData;
			ServerData.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			ServerData.CurrentPlayers = SearchResult.Session.NumOpenPublicConnections;
			ServerData.HostUserName = SearchResult.Session.OwningUserName;

			FString ServerName;
			if (SearchResult.Session.SessionSettings.Get(SESSION_SETTINGS_KEY, ServerName))
				ServerData.Name = ServerName;
			else
				UE_LOG(LogTemp, Warning, TEXT("Session Name Not Found"));

			ServerNames.Add(ServerData);
		}

		MainMenu->SetServerList(ServerNames);

		UE_LOG(LogTemp, Warning, TEXT("Finished Finding Session"));

	}
}
void UMyGameInstance::OnJoinSessionComplate(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult)
{
	if (SessionInterface.IsValid() == false) return;

	FString Address;//�ش� ���� �������ּ�
	if (!SessionInterface->GetResolvedConnectString(InSessionName, Address))
	{
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
		UE_LOG(LogTemp, Error, TEXT("Join Session Failed: %s"), *ErrorMessage);
	}
	LoadMainMenu();
	return;
	}
	UEngine* Engine = GetEngine();
	if (!Engine) return;
	Engine->AddOnScreenDebugMessage(0,5,FColor::Green,FString::Printf(TEXT("Joining To %s"),*Address));
	
	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;
	PC->ClientTravel(Address,ETravelType::TRAVEL_Absolute);
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
