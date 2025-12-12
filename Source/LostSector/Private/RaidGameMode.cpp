// Copyright Epic Games, Inc. All Rights Reserved.

#include "RaidGameMode.h"
#include "Engine/World.h"

ARaidGameMode::ARaidGameMode()
{
}

void ARaidGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		UE_LOG(LogTemp, Warning, TEXT("🎮 [RaidGameMode] 초기화 완료!"));
		UE_LOG(LogTemp, Warning, TEXT("  - 맵 이름: %s"), *GetWorld()->GetMapName());
		UE_LOG(LogTemp, Warning, TEXT("  - 자동 게임 시작 기능 비활성화됨"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	}
}

void ARaidGameMode::CheckAndStartGame()
{
	// 게임맵에서는 자동 게임 시작 기능을 비활성화
	// 이 함수를 오버라이드하여 아무것도 하지 않도록 함
	UE_LOG(LogTemp, Log, TEXT("[RaidGameMode] CheckAndStartGame 호출됨 - 게임맵에서는 자동 시작 기능 비활성화되어 무시됨"));
	// 부모 클래스의 CheckAndStartGame을 호출하지 않음
}
