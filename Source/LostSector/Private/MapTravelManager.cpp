// MapTravelManager.cpp
#include "MapTravelManager.h"
#include "Kismet/GameplayStatics.h"

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