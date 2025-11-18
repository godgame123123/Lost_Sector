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

    // 최대 인원 체크
    if (GetNumPlayers() >= MaxPlayers)
    {
        ErrorMessage = FString::Printf(TEXT("Lobby is full. Maximum %d players allowed."), MaxPlayers);
        return;
    }
}

ALobbyGameMode::ALobbyGameMode()
{
    bUseSeamlessTravel = true;

    // Default Pawn Class 설정
    static ConstructorHelpers::FClassFinder<APawn> PawnClassFinder(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PawnClassFinder.Succeeded())
    {
        DefaultPawnClass = PawnClassFinder.Class;
    }
    
    // Player State Class 설정 - MyPlayerState 사용
    PlayerStateClass = AMyPlayerState::StaticClass();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer)
    {
        // MyPlayerState는 BeginPlay에서 자동으로 데이터를 로드하므로
        // 여기서는 로그만 남기고 인원 체크를 진행합니다
        if (AMyPlayerState* MyPS = NewPlayer->GetPlayerState<AMyPlayerState>())
        {
            FString PlayerID = MyPS->GetUniqueId().IsValid() 
                ? MyPS->GetUniqueId()->ToString() 
                : FString::Printf(TEXT("Local_%d"), MyPS->GetPlayerId());
            
            UE_LOG(LogTemp, Log, TEXT("Player %s joined lobby. MyPlayerState will load data automatically."), *PlayerID);
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
            
            UE_LOG(LogTemp, Log, TEXT("Player %s left lobby. Data saved."), *PlayerID);
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
