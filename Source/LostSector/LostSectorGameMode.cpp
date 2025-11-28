// Copyright Epic Games, Inc. All Rights Reserved.

#include "LostSectorGameMode.h"
#include "LostSectorCharacter.h"
#include "Private/MyPlayerState.h"
#include "UObject/ConstructorHelpers.h"
#include "InventorySaveManager.h"
#include "InventoryComponent.h"
#include "GameFramework/PlayerState.h"
#include "MapTravelManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

ALostSectorGameMode::ALostSectorGameMode()
{
	bUseSeamlessTravel = true;

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
}

void ALostSectorGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	// 최대 인원 체크
	if (GetNumPlayers() >= MaxPlayers)
	{
		ErrorMessage = FString::Printf(TEXT("Lobby is full. Maximum %d players allowed."), MaxPlayers);
		return;
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
				// 인벤토리 슬롯 복원
				InventoryComp->Slots = LoadedData.InventorySlots;
				UE_LOG(LogTemp, Log, TEXT("✅ Inventory restored: %d slots"), LoadedData.InventorySlots.Num());
				
				// 창고 슬롯 복원
				InventoryComp->StorageSlots = LoadedData.StorageSlots;
				UE_LOG(LogTemp, Log, TEXT("✅ Storage restored: %d slots"), LoadedData.StorageSlots.Num());
				
				// JSON에서 로드한 후 Item 포인터가 nullptr이므로 ItemId로 복원
				InventoryComp->RestoreItemPointers();
				InventoryComp->RestoreStorageItemPointers();
				
				// UI 업데이트
				InventoryComp->BroadcastUpdated();
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
			InventoryComp->InitStorageSlots();
		}
	}

	// 서버에서만 인원 체크 및 게임 시작
	if (HasAuthority() && !bGameStarting)
	{
		int32 CurrentPlayers = GetNumPlayers();
		UE_LOG(LogTemp, Log, TEXT("Player joined. Current players: %d / Min: %d / Max: %d"), 
			CurrentPlayers, MinPlayersToStart, MaxPlayers);

		// 인원이 충분하면 게임 시작 체크
		CheckAndStartGame();
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

			// MyPlayerState 데이터 저장 (ServerDataManager 사용)
			if (AMyPlayerState* MyPS = PC->GetPlayerState<AMyPlayerState>())
			{
				MyPS->SavePlayerDataToServer();
				UE_LOG(LogTemp, Log, TEXT("💾 MyPlayerState data saved on logout: %s"), *PlayerID);
			}

			// InventoryComponent 데이터 저장 (InventorySaveManager 사용)
			if (UInventoryComponent* InventoryComp = PC->GetPawn()->FindComponentByClass<UInventoryComponent>())
			{
				// ItemId 설정 (저장 전에 Item 포인터에서 ItemId 추출)
				for (FItemStack& Stack : InventoryComp->Slots)
				{
					if (Stack.Item && Stack.ItemId == NAME_None)
					{
						Stack.ItemId = Stack.Item->ItemId;
					}
				}
				for (FItemStack& Stack : InventoryComp->StorageSlots)
				{
					if (Stack.Item && Stack.ItemId == NAME_None)
					{
						Stack.ItemId = Stack.Item->ItemId;
					}
				}
				
				if (UInventorySaveManager::SavePlayerInventory(this, PlayerID, 
					InventoryComp->Slots, InventoryComp->StorageSlots))
				{
					UE_LOG(LogTemp, Log, TEXT("💾 InventoryComponent data saved on logout: %s (Inventory: %d, Storage: %d)"), 
						*PlayerID, InventoryComp->Slots.Num(), InventoryComp->StorageSlots.Num());
				}
			}
		}
	}

	Super::Logout(Exiting);

	// 서버에서만 인원 체크 (플레이어가 나갔을 때)
	if (HasAuthority() && !bGameStarting)
	{
		int32 CurrentPlayers = GetNumPlayers();
		UE_LOG(LogTemp, Log, TEXT("Player left. Current players: %d / Min: %d / Max: %d"), 
			CurrentPlayers, MinPlayersToStart, MaxPlayers);
	}
}

void ALostSectorGameMode::TransitionToFieldMap(APlayerController* PlayerController, const FString& MapName)
{
	if (!PlayerController) return;

	// 데이터 저장 후 필드맵으로 이동
	if (AMyPlayerState* MyPS = PlayerController->GetPlayerState<AMyPlayerState>())
	{
		MyPS->SavePlayerDataToServer();
	}
	
	// 클라이언트를 필드맵으로 이동
	PlayerController->ClientTravel(MapName, TRAVEL_Absolute);
}

void ALostSectorGameMode::CheckAndStartGame()
{
	if (bGameStarting) return; // 이미 게임 시작 중이면 무시

	int32 CurrentPlayers = GetNumPlayers();
	
	// 기존 타이머가 있으면 취소 (인원 변경 시 타이머 리셋)
	if (StartGameTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(StartGameTimerHandle);
	}

	// 최소 인원이 충족되었는지 확인
	if (CurrentPlayers >= MinPlayersToStart)
	{
		float DelayTime = 0.0f;
		
		if (CurrentPlayers >= MaxPlayers)
		{
			// 최대 인원(3명)이면 5초 후 시작
			DelayTime = StartDelayWithMaxPlayers;
			UE_LOG(LogTemp, Warning, TEXT("Maximum players (%d) reached! Starting game in %.1f seconds..."), 
				MaxPlayers, DelayTime);
		}
		else if (CurrentPlayers == MinPlayersToStart)
		{
			// 최소 인원(2명)이면 3분 후 시작 (추가 인원이 없으면)
			DelayTime = StartDelayWith2Players;
			UE_LOG(LogTemp, Warning, TEXT("Minimum players (%d) reached! Starting game in %.1f seconds if no more players join..."), 
				MinPlayersToStart, DelayTime);
		}

		if (DelayTime > 0.0f)
		{
			GetWorldTimerManager().SetTimer(
				StartGameTimerHandle,
				this,
				&ALostSectorGameMode::TravelToGameMap,
				DelayTime,
				false
			);
		}
		else
		{
			// 대기 시간이 0이면 즉시 시작
			TravelToGameMap();
		}
	}
	else
	{
		// 인원이 부족하면 대기
		UE_LOG(LogTemp, Log, TEXT("Not enough players (%d/%d). Waiting for more..."), 
			CurrentPlayers, MinPlayersToStart);
	}
}

void ALostSectorGameMode::TravelToGameMap()
{
	if (bGameStarting) return; // 중복 실행 방지
	
	bGameStarting = true;
	int32 CurrentPlayers = GetNumPlayers();
	
	UE_LOG(LogTemp, Warning, TEXT("Starting game with %d players!"), CurrentPlayers);
	
	// 모든 플레이어 데이터 저장 (게임 시작 전)
	if (HasAuthority())
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PC = It->Get())
			{
				if (AMyPlayerState* MyPS = PC->GetPlayerState<AMyPlayerState>())
				{
					MyPS->SavePlayerDataToServer();
				}
			}
		}
		UE_LOG(LogTemp, Log, TEXT("All player data saved before game start."));
	}
	
	// 모든 플레이어에게 게임 시작 알림 (선택사항)
	if (UEngine* Engine = GetGameInstance()->GetEngine())
	{
		Engine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Game starting!"));
	}

	// MapTravelManager를 통해 게임 맵으로 이동
	if (UMapTravelManager* TravelManager = GetGameInstance()->GetSubsystem<UMapTravelManager>())
	{
		TravelManager->TravelFromRobbyToGame();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MapTravelManager not found! Cannot travel to game map."));
		bGameStarting = false; // 실패 시 플래그 리셋
	}
}

void ALostSectorGameMode::TestLobbyStatus()
{
	int32 CurrentPlayers = GetNumPlayers();
	float RemainingTime = 0.0f;
	
	if (StartGameTimerHandle.IsValid())
	{
		RemainingTime = GetWorldTimerManager().GetTimerRemaining(StartGameTimerHandle);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("=== Lobby Status ==="));
	UE_LOG(LogTemp, Warning, TEXT("Current Players: %d"), CurrentPlayers);
	UE_LOG(LogTemp, Warning, TEXT("Min Players to Start: %d"), MinPlayersToStart);
	UE_LOG(LogTemp, Warning, TEXT("Max Players: %d"), MaxPlayers);
	UE_LOG(LogTemp, Warning, TEXT("Game Starting: %s"), bGameStarting ? TEXT("Yes") : TEXT("No"));
	
	if (StartGameTimerHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Timer Active: Yes (%.1f seconds remaining)"), RemainingTime);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Timer Active: No"));
	}
	
	if (CurrentPlayers >= MinPlayersToStart)
	{
		if (CurrentPlayers >= MaxPlayers)
		{
			UE_LOG(LogTemp, Warning, TEXT("Status: Maximum players reached - will start in %.1f seconds"), 
				RemainingTime > 0 ? RemainingTime : StartDelayWithMaxPlayers);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Status: Minimum players reached - will start in %.1f seconds if no more join"), 
				RemainingTime > 0 ? RemainingTime : StartDelayWith2Players);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Status: Waiting for more players (%d/%d)"), CurrentPlayers, MinPlayersToStart);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("==================="));
}
