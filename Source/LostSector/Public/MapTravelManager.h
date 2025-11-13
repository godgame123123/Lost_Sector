// MapTravelManager.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MapTravelManager.generated.h"

UCLASS()
class LOSTSECTOR_API UMapTravelManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 서브시스템 초기화
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 로비로 이동
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void TravelToLobby();

	// 게임플레이 맵으로 이동
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void TravelToGameMap(const FString& MapName);

	// RobbyMap에서 GameMap으로 이동
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void TravelFromRobbyToGame();

private:
	// 콘솔 명령어 핸들러
	static void ConsoleCommand_TravelToGame(const TArray<FString>& Args, UWorld* World);
	static void ConsoleCommand_TravelToLobby(const TArray<FString>& Args, UWorld* World);
};