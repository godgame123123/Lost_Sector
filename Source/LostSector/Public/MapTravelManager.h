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
	// 로비로 이동
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void TravelToLobby();

	// 게임플레이 맵으로 이동
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void TravelToGameMap(const FString& MapName);
};