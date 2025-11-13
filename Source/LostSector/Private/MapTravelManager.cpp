// MapTravelManager.cpp
#include "MapTravelManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

namespace
{
	const TCHAR* LobbyMapPath = TEXT("/Game/BattleRoyaleStarterKit/Maps/BattleRoyale_Map_a/RobbyMap");
	const TCHAR* LobbyTravelOptions = TEXT("?listen");

	const TCHAR* GameMapPath = TEXT("/Game/BattleRoyaleStarterKit/Maps/BattleRoyale_Map_a/GameMap");
	const TCHAR* GameTravelOptions = TEXT("?game=/Script/LostSector.RaidGameMode");

	FString BuildServerTravelURL(const TCHAR* MapPath, const TCHAR* Options)
	{
		FString URL(MapPath);
		if (Options && *Options)
		{
			URL += Options;
		}
		return URL;
	}

	bool TravelOnServer(UWorld* World, const FString& URL)
	{
		const ENetMode NetMode = World->GetNetMode();
		if (NetMode == NM_DedicatedServer || NetMode == NM_ListenServer)
		{
			World->ServerTravel(URL);
			return true;
		}

		return false;
	}
}

void UMapTravelManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 콘솔 명령어 등록
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("TravelToGame"),
		TEXT("RobbyMap에서 GameMap으로 이동합니다. 사용법: TravelToGame"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UMapTravelManager::ConsoleCommand_TravelToGame),
		ECVF_Default
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("TravelToLobby"),
		TEXT("로비맵으로 이동합니다. 사용법: TravelToLobby"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UMapTravelManager::ConsoleCommand_TravelToLobby),
		ECVF_Default
	);

	UE_LOG(LogTemp, Log, TEXT("MapTravelManager: 콘솔 명령어 등록 완료 (TravelToGame, TravelToLobby)"));
}

void UMapTravelManager::Deinitialize()
{
	// 콘솔 명령어 해제
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("TravelToGame"));
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("TravelToLobby"));

	Super::Deinitialize();
}

void UMapTravelManager::TravelToLobby()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const ENetMode NetMode = World->GetNetMode();

	if (NetMode == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("TravelToLobby는 서버 또는 스탠드얼론 환경에서만 호출해야 합니다."));
		return;
	}

	if (NetMode == NM_Standalone)
	{
		UGameplayStatics::OpenLevel(World, FName(LobbyMapPath));
		return;
	}

	const FString LobbyURL = BuildServerTravelURL(LobbyMapPath, LobbyTravelOptions);
	UE_LOG(LogTemp, Log, TEXT("ServerTravel -> %s"), *LobbyURL);
	TravelOnServer(World, LobbyURL);
}

void UMapTravelManager::TravelToGameMap(const FString& MapName)
{
	UWorld* World = GetWorld();
	if (!World) return;

	const ENetMode NetMode = World->GetNetMode();

	if (NetMode == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("TravelToGameMap은 서버 또는 스탠드얼론 환경에서만 호출해야 합니다."));
		return;
	}

	if (NetMode == NM_Standalone)
	{
		UGameplayStatics::OpenLevel(World, FName(*MapName));
		return;
	}

	const FString TargetURL = MapName.Contains(TEXT("?")) ? MapName : BuildServerTravelURL(*MapName, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("ServerTravel -> %s"), *TargetURL);
	TravelOnServer(World, TargetURL);
}

void UMapTravelManager::TravelFromRobbyToGame()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const ENetMode NetMode = World->GetNetMode();

	if (NetMode == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("TravelFromRobbyToGame은 서버에서 호출해야 합니다. 클라이언트에서는 자동으로 따라갑니다."));
		return;
	}

	if (NetMode == NM_Standalone)
	{
		UGameplayStatics::OpenLevel(World, FName(GameMapPath));
		return;
	}

	const FString GameURL = BuildServerTravelURL(GameMapPath, GameTravelOptions);
	UE_LOG(LogTemp, Log, TEXT("ServerTravel -> %s"), *GameURL);
	TravelOnServer(World, GameURL);
}

void UMapTravelManager::ConsoleCommand_TravelToGame(const TArray<FString>& Args, UWorld* World)
{
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("World가 유효하지 않습니다."));
		return;
	}

	if (UMapTravelManager* TravelManager = World->GetGameInstance()->GetSubsystem<UMapTravelManager>())
	{
		ENetMode NetMode = World->GetNetMode();
		if (NetMode == NM_Client)
		{
			UE_LOG(LogTemp, Warning, TEXT("TravelToGame 콘솔 명령은 서버 콘솔에서만 사용 가능합니다."));
			return;
		}

		TravelManager->TravelFromRobbyToGame();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MapTravelManager를 찾을 수 없습니다."));
	}
}

void UMapTravelManager::ConsoleCommand_TravelToLobby(const TArray<FString>& Args, UWorld* World)
{
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("World가 유효하지 않습니다."));
		return;
	}

	if (UMapTravelManager* TravelManager = World->GetGameInstance()->GetSubsystem<UMapTravelManager>())
	{
		ENetMode NetMode = World->GetNetMode();
		if (NetMode == NM_Client)
		{
			UE_LOG(LogTemp, Warning, TEXT("TravelToLobby 콘솔 명령은 서버 콘솔에서만 사용 가능합니다."));
			return;
		}

		TravelManager->TravelToLobby();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MapTravelManager를 찾을 수 없습니다."));
	}
}