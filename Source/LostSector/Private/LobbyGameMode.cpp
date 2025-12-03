#include "LobbyGameMode.h"

#include "ServerDataManager.h"
#include "MyPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "MapTravelManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"


void ALobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
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

    // 최대 인원 체크
    if (GetNumPlayers() >= MaxPlayers)
    {
        FString FullMessage = FString::Printf(TEXT("Lobby is full. Maximum %d players allowed."), MaxPlayers);
        ErrorMessage = FullMessage;
        if (HasAuthority())
        {
            UE_LOG(LogTemp, Error, TEXT("[서버 조인 시도] ❌ 서버가 가득 찼습니다! - %s"), *FullMessage);
            UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
        }
        return;
    }
    
    if (HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("[서버 조인 시도] ✅ 조인 허용됨"));
        UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
    }
}

ALobbyGameMode::ALobbyGameMode()
{
    bUseSeamlessTravel = true;

    // Default Pawn Class 설정 (블루프린트에서 설정하지 않은 경우에만 기본값 사용)
    if (DefaultPawnClass == nullptr)
    {
        static ConstructorHelpers::FClassFinder<APawn> PawnClassFinder(TEXT("/Game/Team_Folder/Kimjaehwan/Player/BP_KimjaehwanCharacter"));
        if (PawnClassFinder.Succeeded())
        {
            DefaultPawnClass = PawnClassFinder.Class;
        }
    }
    
    // Player State Class 설정 - MyPlayerState 사용
    PlayerStateClass = AMyPlayerState::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
        UE_LOG(LogTemp, Warning, TEXT("🎮 [LobbyGameMode] 초기화 완료!"));
        UE_LOG(LogTemp, Warning, TEXT("  - 맵 이름: %s"), *GetWorld()->GetMapName());
        UE_LOG(LogTemp, Warning, TEXT("  - 최소 인원: %d"), MinPlayersToStart);
        UE_LOG(LogTemp, Warning, TEXT("  - 최대 인원: %d"), MaxPlayers);
        UE_LOG(LogTemp, Warning, TEXT("  - 현재 플레이어 수: %d"), GetNumPlayers());
        UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
    }
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // PostLogin이 호출되었는지 확인하는 로그 (항상 출력)
    UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] PostLogin 호출됨 - PlayerController: %s"), 
        NewPlayer ? *GetNameSafe(NewPlayer) : TEXT("NULL"));

    if (NewPlayer)
    {
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
            
            // MyPlayerState는 BeginPlay에서 자동으로 데이터를 로드하므로
            // 여기서는 로그만 남기고 인원 체크를 진행합니다
            if (AMyPlayerState* MyPS = NewPlayer->GetPlayerState<AMyPlayerState>())
            {
                FString PlayerID = MyPS->GetUniqueId().IsValid() 
                    ? MyPS->GetUniqueId()->ToString() 
                    : FString::Printf(TEXT("Local_%d"), MyPS->GetPlayerId());
                
                FString PlayerName = NewPlayer->PlayerState ? NewPlayer->PlayerState->GetPlayerName() : TEXT("Unknown");
                
                if (bIsHost)
                {
                    UE_LOG(LogTemp, Warning, TEXT("  └─ 호스트 이름: %s"), *PlayerName);
                    UE_LOG(LogTemp, Warning, TEXT("  └─ 호스트 ID: %s"), *PlayerID);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("  └─ 플레이어 이름: %s"), *PlayerName);
                    UE_LOG(LogTemp, Warning, TEXT("  └─ 플레이어 ID: %s"), *PlayerID);
                    UE_LOG(LogTemp, Warning, TEXT("  └─ PlayerController: %s"), *GetNameSafe(NewPlayer));
                }
                UE_LOG(LogTemp, Warning, TEXT("  └─ MyPlayerState: %s"), *GetNameSafe(MyPS));
                UE_LOG(LogTemp, Log, TEXT("  └─ MyPlayerState가 자동으로 데이터를 로드합니다."));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[서버 입장] ❌ MyPlayerState를 찾을 수 없습니다!"));
            }

            if (!bGameStarting)
            {
                int32 CurrentPlayers = GetNumPlayers();
                if (bIsHost)
                {
                    UE_LOG(LogTemp, Warning, TEXT("🎮 [호스트] 현재 서버 상태:"));
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("👥 [호스트 확인] 현재 서버 상태 (호스트에게 표시):"));
                }
                UE_LOG(LogTemp, Warning, TEXT("  └─ 현재 플레이어 수: %d"), CurrentPlayers);
                UE_LOG(LogTemp, Warning, TEXT("  └─ 최소 인원: %d"), MinPlayersToStart);
                UE_LOG(LogTemp, Warning, TEXT("  └─ 최대 인원: %d"), MaxPlayers);
                UE_LOG(LogTemp, Warning, TEXT("  └─ 빈 슬롯: %d"), MaxPlayers - CurrentPlayers);

                // 인원이 충분하면 게임 시작 체크
                CheckAndStartGame();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[서버 입장] ⚠️ 게임이 이미 시작 중입니다."));
            }
            
            UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
        }
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    // 로비 나갈 때 데이터 저장
    if (APlayerController* PC = Cast<APlayerController>(Exiting))
    {
        if (AMyPlayerState* MyPS = PC->GetPlayerState<AMyPlayerState>())
        {
            // MyPlayerState의 저장 함수 호출
            MyPS->SavePlayerDataToServer();
            
            FString PlayerID = MyPS->GetUniqueId().IsValid() 
                ? MyPS->GetUniqueId()->ToString() 
                : FString::Printf(TEXT("Local_%d"), MyPS->GetPlayerId());
            
            FString PlayerName = PC->PlayerState ? PC->PlayerState->GetPlayerName() : TEXT("Unknown");
            
            if (HasAuthority())
            {
                UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
                UE_LOG(LogTemp, Warning, TEXT("👋 [플레이어 퇴장] 플레이어가 서버를 떠났습니다 (호스트에게 표시)"));
                UE_LOG(LogTemp, Warning, TEXT("  └─ 플레이어 이름: %s"), *PlayerName);
                UE_LOG(LogTemp, Warning, TEXT("  └─ 플레이어 ID: %s"), *PlayerID);
                UE_LOG(LogTemp, Warning, TEXT("  └─ 플레이어 데이터 저장 완료"));
            }
        }
    }

    Super::Logout(Exiting);

    // 서버에서만 인원 체크 (플레이어가 나갔을 때)
    if (HasAuthority() && !bGameStarting)
    {
        int32 CurrentPlayers = GetNumPlayers();
        UE_LOG(LogTemp, Warning, TEXT("👥 [호스트 확인] 현재 서버 상태 (호스트에게 표시):"));
        UE_LOG(LogTemp, Warning, TEXT("  └─ 현재 플레이어 수: %d"), CurrentPlayers);
        UE_LOG(LogTemp, Warning, TEXT("  └─ 최소 인원: %d"), MinPlayersToStart);
        UE_LOG(LogTemp, Warning, TEXT("  └─ 최대 인원: %d"), MaxPlayers);
        UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════════════════════"));
    }
}

void ALobbyGameMode::TransitionToFieldMap(APlayerController* PlayerController, const FString& MapName)
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

void ALobbyGameMode::CheckAndStartGame()
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
                &ALobbyGameMode::TravelToGameMap,
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

void ALobbyGameMode::TravelToGameMap()
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

void ALobbyGameMode::TestLobbyStatus()
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
