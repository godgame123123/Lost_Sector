// Copyright Epic Games, Inc. All Rights Reserved.

#include "LostSectorGameMode.h"
#include "../LostSectorCharacter.h"
#include "MyPlayerState.h"
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
	
	// 최대 인원을 3명(호스트 포함)으로 강제 설정
	MaxPlayers = 3;
}

void ALostSectorGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// 최대 인원을 3명(호스트 포함)으로 강제 설정 (블루프린트 설정 무시)
		if (MaxPlayers != 3)
		{
			UE_LOG(LogTemp, Warning, TEXT("⚠️ [LostSectorGameMode] MaxPlayers가 3이 아닙니다! %d -> 3으로 강제 변경"), MaxPlayers);
			MaxPlayers = 3;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		UE_LOG(LogTemp, Warning, TEXT("🎮 [LostSectorGameMode] 초기화 완료!"));
		UE_LOG(LogTemp, Warning, TEXT("  - 맵 이름: %s"), *GetWorld()->GetMapName());
		UE_LOG(LogTemp, Warning, TEXT("  - 최소 인원: %d"), MinPlayersToStart);
		UE_LOG(LogTemp, Warning, TEXT("  - 최대 인원: %d (호스트 포함, 강제 설정)"), MaxPlayers);
		UE_LOG(LogTemp, Warning, TEXT("  - 현재 플레이어 수: %d"), GetNumPlayers());
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	}

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

	// 서버에서만 로그 출력
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		UE_LOG(LogTemp, Warning, TEXT("[서버 조인 시도] 플레이어가 서버 조인을 시도했습니다"));
		UE_LOG(LogTemp, Warning, TEXT("  - 주소: %s"), *Address);
		UE_LOG(LogTemp, Warning, TEXT("  - UniqueId: %s"), UniqueId.IsValid() ? *UniqueId.ToString() : TEXT("Invalid"));
		UE_LOG(LogTemp, Warning, TEXT("  - 현재 플레이어 수: %d/%d"), GetNumPlayers(), MaxPlayers);
	}

	// 최대 인원 체크 (조인하려는 플레이어를 포함해서 체크)
	// PreLogin이 호출될 때는 아직 플레이어가 접속하기 전이므로, 조인하려는 플레이어를 포함해서 체크해야 함
	int32 CurrentPlayers = GetNumPlayers();
	int32 PlayersAfterJoin = CurrentPlayers + 1; // 조인 시도 플레이어 포함
	
	// MaxPlayers는 호스트를 포함한 최대 인원이므로, >= 로 체크해야 함
	if (PlayersAfterJoin > MaxPlayers)
	{
		FString FullMessage = FString::Printf(TEXT("Lobby is full. Maximum %d players allowed (including host)."), MaxPlayers);
		ErrorMessage = FullMessage;
		if (HasAuthority())
		{
			UE_LOG(LogTemp, Error, TEXT("[서버 조인 시도] ❌ 서버가 가득 찼습니다! - %s"), *FullMessage);
			UE_LOG(LogTemp, Error, TEXT("  └─ 현재 플레이어: %d, 최대 인원: %d (호스트 포함)"), CurrentPlayers, MaxPlayers);
			UE_LOG(LogTemp, Error, TEXT("  └─ 조인 시도 플레이어 포함 시: %d명 (최대 인원 초과!)"), PlayersAfterJoin);
			UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		}
		return;
	}
	
	// 추가 안전 체크: 정확히 MaxPlayers에 도달했는지 확인
	if (PlayersAfterJoin == MaxPlayers)
	{
		if (HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("[서버 조인 시도] ⚠️ 최대 인원에 도달합니다! (현재: %d, 조인 후: %d/%d)"), 
				CurrentPlayers, PlayersAfterJoin, MaxPlayers);
		}
	}
	
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[서버 조인 시도] ✅ 조인 허용됨"));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
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

	// PostLogin이 호출되었는지 확인하는 로그 (항상 출력)
	UE_LOG(LogTemp, Warning, TEXT("[LostSectorGameMode] PostLogin 호출됨 - PlayerController: %s"), 
		NewPlayer ? *GetNameSafe(NewPlayer) : TEXT("NULL"));

	// 서버에서만 상세 로그 출력
	if (HasAuthority())
	{
		// IsLocalController()를 사용하여 호스트인지 정확히 판단
		bool bIsHost = NewPlayer->IsLocalController();
		int32 CurrentPlayerCount = GetNumPlayers();
		
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		if (bIsHost)
		{
			UE_LOG(LogTemp, Warning, TEXT("🎮 [호스트 입장] 서버 호스트가 서버에 입장했습니다!"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("👤 [플레이어 입장] 새로운 플레이어가 서버에 조인했습니다!"));
			UE_LOG(LogTemp, Warning, TEXT("  └─ ⚠️ 호스트가 이 로그를 확인할 수 있습니다!"));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("  └─ 플레이어 이름: %s"), *NewPlayer->PlayerState->GetPlayerName());
		UE_LOG(LogTemp, Warning, TEXT("  └─ 플레이어 ID: %s"), *PlayerID);
		UE_LOG(LogTemp, Warning, TEXT("  └─ PlayerController: %s"), *GetNameSafe(NewPlayer));
		
		// 서버 상태 정보
		UE_LOG(LogTemp, Warning, TEXT("👥 [호스트 확인] 현재 서버 상태 (호스트에게 표시):"));
		UE_LOG(LogTemp, Warning, TEXT("  └─ 현재 플레이어 수: %d"), CurrentPlayerCount);
		UE_LOG(LogTemp, Warning, TEXT("  └─ 최소 인원: %d"), MinPlayersToStart);
		UE_LOG(LogTemp, Warning, TEXT("  └─ 최대 인원: %d"), MaxPlayers);
		UE_LOG(LogTemp, Warning, TEXT("  └─ 빈 슬롯: %d"), MaxPlayers - CurrentPlayerCount);
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Player joined: %s (ID: %s)"), 
			*NewPlayer->PlayerState->GetPlayerName(), *PlayerID);
	}

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
	}

	// 서버에서만 인원 체크 및 게임 시작
	if (HasAuthority())
	{
		// 게임맵에서는 CheckAndStartGame을 호출하지 않음
		FString CurrentMapName = GetWorld()->GetMapName();
		if (CurrentMapName.Contains(TEXT("GameMap")))
		{
			UE_LOG(LogTemp, Log, TEXT("[PostLogin] 게임맵에서는 CheckAndStartGame을 호출하지 않습니다. 맵 이름: %s"), *CurrentMapName);
		}
		else
		{
			// PostLogin이 호출된 후에는 새로운 플레이어가 이미 카운트에 포함되어 있음
			int32 PlayerCountAfterLogin = GetNumPlayers();
			
			if (bGameStarting)
			{
				UE_LOG(LogTemp, Warning, TEXT("[PostLogin] ⚠️ 게임이 이미 시작 중입니다. CheckAndStartGame 호출 건너뜀 (현재 플레이어: %d)"), PlayerCountAfterLogin);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[PostLogin] ✅ CheckAndStartGame 호출 (현재 플레이어: %d/%d)"), PlayerCountAfterLogin, MaxPlayers);
				// 인원이 충분하면 게임 시작 체크
				CheckAndStartGame();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[PostLogin] 클라이언트에서는 CheckAndStartGame을 호출하지 않습니다."));
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
	if (HasAuthority())
	{
		FString PlayerName = TEXT("Unknown");
		FString PlayerID = TEXT("Unknown ID");
		
		if (APlayerController* PC = Cast<APlayerController>(Exiting))
		{
			if (PC->PlayerState)
			{
				PlayerName = PC->PlayerState->GetPlayerName();
				if (PC->PlayerState->GetUniqueId().IsValid())
				{
					PlayerID = PC->PlayerState->GetUniqueId()->ToString();
				}
				else
				{
					PlayerID = FString::Printf(TEXT("Local_%d"), PC->PlayerState->GetPlayerId());
				}
			}
		}
		
		int32 CurrentPlayers = GetNumPlayers();
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		UE_LOG(LogTemp, Warning, TEXT("[플레이어 퇴장] 플레이어가 서버를 떠났습니다 (호스트에게 표시)!"));
		UE_LOG(LogTemp, Warning, TEXT("  └─ 플레이어 이름: %s"), *PlayerName);
		UE_LOG(LogTemp, Warning, TEXT("  └─ 플레이어 ID: %s"), *PlayerID);
		
		UE_LOG(LogTemp, Warning, TEXT("👥 [호스트 확인] 현재 서버 상태 (호스트에게 표시):"));
		UE_LOG(LogTemp, Warning, TEXT("  └─ 현재 플레이어 수: %d"), CurrentPlayers);
		UE_LOG(LogTemp, Warning, TEXT("  └─ 최소 인원: %d"), MinPlayersToStart);
		UE_LOG(LogTemp, Warning, TEXT("  └─ 최대 인원: %d"), MaxPlayers);
		UE_LOG(LogTemp, Warning, TEXT("  └─ 빈 슬롯: %d"), MaxPlayers - CurrentPlayers);
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
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
	if (bGameStarting)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CheckAndStartGame] ⚠️ 게임이 이미 시작 중입니다. 무시합니다."));
		return; // 이미 게임 시작 중이면 무시
	}

	int32 CurrentPlayers = GetNumPlayers();
	
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	UE_LOG(LogTemp, Warning, TEXT("[CheckAndStartGame] 게임 시작 조건 확인"));
	UE_LOG(LogTemp, Warning, TEXT("  └─ 현재 플레이어 수: %d"), CurrentPlayers);
	UE_LOG(LogTemp, Warning, TEXT("  └─ 최소 인원: %d"), MinPlayersToStart);
	UE_LOG(LogTemp, Warning, TEXT("  └─ 최대 인원: %d"), MaxPlayers);
	UE_LOG(LogTemp, Warning, TEXT("  └─ StartDelayWith2Players: %.1f초"), StartDelayWith2Players);
	UE_LOG(LogTemp, Warning, TEXT("  └─ StartDelayWithMaxPlayers: %.1f초"), StartDelayWithMaxPlayers);
	
	// 기존 타이머가 있으면 취소 (인원 변경 시 타이머 리셋)
	if (StartGameTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(StartGameTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("[CheckAndStartGame] 기존 타이머 취소됨"));
	}

	// 최소 인원이 충족되었는지 확인
	if (CurrentPlayers >= MinPlayersToStart)
	{
		float DelayTime = 0.0f;
		bool bShouldStart = false;
		
		if (CurrentPlayers >= MaxPlayers)
		{
			// 최대 인원(3명)이면 설정된 시간 후 시작
			DelayTime = StartDelayWithMaxPlayers;
			bShouldStart = true;
			UE_LOG(LogTemp, Warning, TEXT("[CheckAndStartGame] ✅ 최대 인원(%d명) 도달! %.1f초 후 게임 시작..."), 
				MaxPlayers, DelayTime);
		}
		else if (CurrentPlayers >= MinPlayersToStart)
		{
			// 최소 인원(2명) 이상이면 설정된 시간 후 시작 (추가 인원이 없으면)
			DelayTime = StartDelayWith2Players;
			bShouldStart = true;
			UE_LOG(LogTemp, Warning, TEXT("[CheckAndStartGame] ✅ 최소 인원(%d명) 이상 도달! %.1f초 후 게임 시작 (추가 인원이 없으면)..."), 
				CurrentPlayers, DelayTime);
		}

		if (bShouldStart)
		{
			if (DelayTime > 0.0f)
			{
				UE_LOG(LogTemp, Warning, TEXT("[CheckAndStartGame] ⏱️ 타이머 설정: %.1f초 후 게임 시작"), DelayTime);
				GetWorldTimerManager().SetTimer(
					StartGameTimerHandle,
					this,
					&ALostSectorGameMode::TravelToGameMap,
					DelayTime,
					false
				);
				UE_LOG(LogTemp, Warning, TEXT("[CheckAndStartGame] ✅ 타이머가 성공적으로 설정되었습니다."));
			}
			else
			{
				// 대기 시간이 0이면 즉시 시작
				UE_LOG(LogTemp, Warning, TEXT("[CheckAndStartGame] 🚀 대기 시간이 0이므로 즉시 게임 시작!"));
				TravelToGameMap();
			}
		}
	}
	else
	{
		// 인원이 부족하면 대기
		UE_LOG(LogTemp, Warning, TEXT("[CheckAndStartGame] ⏳ 인원 부족 (%d/%d). 더 많은 플레이어를 기다립니다..."), 
			CurrentPlayers, MinPlayersToStart);
	}
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
}

void ALostSectorGameMode::TravelToGameMap()
{
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
	UE_LOG(LogTemp, Warning, TEXT("[TravelToGameMap] 게임 맵으로 이동 시작!"));
	
	// 중복 실행 방지: 이미 게임이 시작 중이면 즉시 리턴
	if (bGameStarting)
	{
		UE_LOG(LogTemp, Error, TEXT("[TravelToGameMap] ⚠️ 게임이 이미 시작 중입니다. 중복 실행 방지."));
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return; // 중복 실행 방지
	}
	
	// 플래그를 먼저 설정하여 중복 호출 방지
	bGameStarting = true;
	int32 CurrentPlayers = GetNumPlayers();
	
	UE_LOG(LogTemp, Warning, TEXT("[TravelToGameMap] ✅ 게임 시작! 플레이어 수: %d"), CurrentPlayers);
	
	// 서버에서만 실행
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("[TravelToGameMap] ❌ 서버가 아닙니다! HasAuthority() = false"));
		bGameStarting = false;
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[TravelToGameMap] ❌ World가 유효하지 않습니다!"));
		bGameStarting = false;
		UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
		return;
	}
	
	ENetMode NetMode = World->GetNetMode();
	UE_LOG(LogTemp, Warning, TEXT("[TravelToGameMap] 네트워크 모드: %d (NM_ListenServer=3)"), (int32)NetMode);
	
	// 모든 플레이어 데이터 저장 (게임 시작 전)
	UE_LOG(LogTemp, Warning, TEXT("[TravelToGameMap] 모든 플레이어 데이터 저장 중..."));
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (AMyPlayerState* MyPS = PC->GetPlayerState<AMyPlayerState>())
			{
				MyPS->SavePlayerDataToServer();
				UE_LOG(LogTemp, Log, TEXT("  └─ 플레이어 데이터 저장: %s"), *MyPS->GetPlayerName());
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[TravelToGameMap] ✅ 모든 플레이어 데이터 저장 완료"));
	
	// 모든 플레이어에게 게임 시작 알림
	if (UEngine* Engine = GetGameInstance()->GetEngine())
	{
		Engine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Game starting!"));
		UE_LOG(LogTemp, Warning, TEXT("[TravelToGameMap] 게임 시작 알림 표시"));
	}

	// MapTravelManager를 통해 게임 맵으로 이동
	UE_LOG(LogTemp, Warning, TEXT("[TravelToGameMap] MapTravelManager 찾는 중..."));
	UMapTravelManager* TravelManager = GetGameInstance()->GetSubsystem<UMapTravelManager>();
	if (TravelManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TravelToGameMap] ✅ MapTravelManager 찾음! TravelFromRobbyToGame() 호출"));
		TravelManager->TravelFromRobbyToGame();
		UE_LOG(LogTemp, Warning, TEXT("[TravelToGameMap] 🚀 맵 이동 요청 완료!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[TravelToGameMap] ❌ MapTravelManager를 찾을 수 없습니다!"));
		UE_LOG(LogTemp, Error, TEXT("  └─ GameInstance: %s"), *GetNameSafe(GetGameInstance()));
		UE_LOG(LogTemp, Error, TEXT("  └─ GetSubsystem 실패 - 서브시스템이 등록되지 않았을 수 있습니다."));
		bGameStarting = false; // 실패 시 플래그 리셋
	}
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
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
