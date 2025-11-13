// MapTravelManager.cpp
#include "MapTravelManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

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

	UE_LOG(LogTemp, Log, TEXT("Traveling to Lobby"));
    
	// 로비맵으로 이동 (Listen Server로 열기)
	UGameplayStatics::OpenLevel(World, FName(TEXT("LobbyMap")), true, TEXT("listen"));
}

void UMapTravelManager::TravelToGameMap(const FString& MapName)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UE_LOG(LogTemp, Log, TEXT("Traveling to Game Map: %s"), *MapName);
    
	// 게임플레이 맵으로 이동
	UGameplayStatics::OpenLevel(World, FName(*MapName), true, TEXT("listen"));
}

void UMapTravelManager::TravelFromRobbyToGame()
{
	UWorld* World = GetWorld();
	if (!World) 
	{
		UE_LOG(LogTemp, Error, TEXT("World가 유효하지 않습니다."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Traveling from RobbyMap to GameMap"));
    
	// GameMap으로 이동
	// TravelToLobby와 동일한 방식: 맵 이름만 사용
	// DefaultEngine.ini의 GameModeMapPrefixes에 "GameMap"이 등록되어 있음
	FString MapName = TEXT("GameMap");
	
	// ServerTravel을 항상 사용 (LobbyPlayerController와 동일한 방식)
	// ServerTravel은 서버가 아니어도 작동하며, 더 안정적임
	FString ServerURL = MapName + TEXT("?listen");
	
	// World가 서버 역할을 할 수 있는지 확인
	if (World->GetNetMode() == NM_Standalone || World->GetNetMode() == NM_ListenServer || World->GetNetMode() == NM_DedicatedServer)
	{
		World->ServerTravel(ServerURL);
		UE_LOG(LogTemp, Log, TEXT("ServerTravel 사용: %s"), *ServerURL);
	}
	else
	{
		// 클라이언트 모드인 경우 OpenLevel 사용 (TravelToLobby와 동일한 방식)
		FName LevelName = FName(*MapName);
		if (LevelName.IsNone())
		{
			UE_LOG(LogTemp, Error, TEXT("맵 이름이 None입니다: %s"), *MapName);
			return;
		}
		
		UGameplayStatics::OpenLevel(World, LevelName, true, TEXT("listen"));
		UE_LOG(LogTemp, Log, TEXT("OpenLevel 사용 (Listen Server 모드): %s"), *MapName);
	}
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
		TravelManager->TravelToLobby();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MapTravelManager를 찾을 수 없습니다."));
	}
}