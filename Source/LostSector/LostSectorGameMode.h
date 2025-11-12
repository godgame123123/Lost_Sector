// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LostSectorGameMode.generated.h"

UCLASS(minimalapi)
class ALostSectorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALostSectorGameMode();

protected:
	virtual void BeginPlay() override;
	// ✅ Dedicated Server 전용 초기화
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

public:
	// 플레이어 접속 시
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 플레이어 퇴장 시
	virtual void Logout(AController* Exiting) override;
};



