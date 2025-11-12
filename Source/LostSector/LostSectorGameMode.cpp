// Copyright Epic Games, Inc. All Rights Reserved.

#include "LostSectorGameMode.h"
#include "LostSectorCharacter.h"
#include "Private/MyPlayerState.h"
#include "UObject/ConstructorHelpers.h"
#include "InventorySaveManager.h"
#include "InventoryComponent.h"
#include "GameFramework/PlayerState.h"

ALostSectorGameMode::ALostSectorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PlayerStateClass = AMyPlayerState::StaticClass();
}

void ALostSectorGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 자동 저장 활성화 (5분마다)
	if (UInventorySaveManager* SaveManager = GetGameInstance()->GetSubsystem<UInventorySaveManager>())
	{
		SaveManager->EnableAutoSave(300.0f);
		UE_LOG(LogTemp, Log, TEXT("✅ Auto-save enabled (every 5 minutes)"));
	}
}

void ALostSectorGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	UE_LOG(LogTemp, Log, TEXT("🎮 GameMode initialized on map: %s"), *MapName);
	
	// Dedicated Server인지 확인
	if (GetNetMode() == NM_DedicatedServer)
	{
		UE_LOG(LogTemp, Warning, TEXT("🖥️ Running as Dedicated Server"));
	}
}

void ALostSectorGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer || !NewPlayer->PlayerState)
	{
		return;
	}

	// 플레이어 고유 ID 가져오기
	FString PlayerID;
	if (NewPlayer->PlayerState->GetUniqueId().IsValid())
	{
		PlayerID = NewPlayer->PlayerState->GetUniqueId()->ToString();
	}
	else
	{
		// 로컬 테스트용 ID
		PlayerID = FString::Printf(TEXT("Local_%d"), NewPlayer->PlayerState->GetPlayerId());
	}

	UE_LOG(LogTemp, Log, TEXT("Player joined: %s (ID: %s)"), 
		*NewPlayer->PlayerState->GetPlayerName(), *PlayerID);

	// 인벤토리 데이터 로드
	FPlayerInventorySaveData LoadedData;
	if (UInventorySaveManager::LoadPlayerInventory(this, PlayerID, LoadedData))
	{
		// 기존 데이터 복원
		if (APawn* PlayerPawn = NewPlayer->GetPawn())
		{
			if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
			{
				InventoryComp->Slots = LoadedData.InventorySlots;
				UE_LOG(LogTemp, Log, TEXT("✅ Inventory restored: %d items"), LoadedData.InventorySlots.Num());
				
				// TODO: 창고 데이터도 복원
				// StorageComponent->Slots = LoadedData.StorageSlots;
			}
		}
	}
	else
	{
		// 신규 플레이어 - 빈 인벤토리 초기화
		UE_LOG(LogTemp, Log, TEXT("🆕 New player - creating fresh inventory"));
		
		if (APawn* PlayerPawn = NewPlayer->GetPawn())
		{
			if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
			{
				InventoryComp->InitSlots();
			}
		}
	}
}

void ALostSectorGameMode::Logout(AController* Exiting)
{
	// 플레이어 퇴장 시 인벤토리 저장
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		if (PC->PlayerState && PC->GetPawn())
		{
			FString PlayerID;
			if (PC->PlayerState->GetUniqueId().IsValid())
			{
				PlayerID = PC->PlayerState->GetUniqueId()->ToString();
			}
			else
			{
				PlayerID = FString::Printf(TEXT("Local_%d"), PC->PlayerState->GetPlayerId());
			}

			if (UInventoryComponent* InventoryComp = PC->GetPawn()->FindComponentByClass<UInventoryComponent>())
			{
				// TODO: 창고 데이터도 함께 저장
				TArray<FItemStack> EmptyStorage;
				
				if (UInventorySaveManager::SavePlayerInventory(this, PlayerID, 
					InventoryComp->Slots, EmptyStorage))
				{
					UE_LOG(LogTemp, Log, TEXT("💾 Player inventory saved on logout: %s"), *PlayerID);
				}
			}
		}
	}

	Super::Logout(Exiting);
}
