#include "LobbyGameMode.h"
#include "ServerDataManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

/*
ALobbyGameMode::ALobbyGameMode()
{
    bUseSeamlessTravel = true;
}
*/
void ALobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

    // 로비는 1인 전용
    if (GetNumPlayers() >= 1)
    {
        ErrorMessage = TEXT("Lobby is full. Only one player allowed per lobby instance.");
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
    
    // Player State Class 설정
    static ConstructorHelpers::FClassFinder<APlayerState> PlayerStateClassFinder(TEXT("/Script/Engine.PlayerState"));
    if (PlayerStateClassFinder.Succeeded())
    {
        PlayerStateClass = PlayerStateClassFinder.Class;
    }
}
/*
//void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer)
    {
        // 플레이어 데이터 로드
        FString PlayerID = NewPlayer->GetPlayerState<APlayerState>()->GetUniqueId().ToString();
        
        if (UServerDataManager* DataManager = UServerDataManager::GetInstance(GetWorld()))
        {
            FPlayerData PlayerData;
            if (DataManager->LoadPlayerData(PlayerID, PlayerData))
            {
                UE_LOG(LogTemp, Log, TEXT("Player %s loaded in lobby. Money: %d, Level: %d"), 
                    *PlayerID, PlayerData.Money, PlayerData.Level);
            }
        }
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    // 로비 나갈 때 데이터 저장
    if (APlayerController* PC = Cast<APlayerController>(Exiting))
    {
        FString PlayerID = PC->GetPlayerState<APlayerState>()->GetUniqueId().ToString();
        
        // 여기서 현재 플레이어 데이터를 저장
        // (실제 데이터는 PlayerController나 PlayerState에서 가져와야 함)
    }

    Super::Logout(Exiting);
}

void ALobbyGameMode::TransitionToFieldMap(APlayerController* PlayerController, const FString& MapName)
{
    if (!PlayerController) return;

    // 데이터 저장 후 필드맵으로 이동
    FString PlayerID = PlayerController->GetPlayerState<APlayerState>()->GetUniqueId().ToString();
    
    // 클라이언트를 필드맵으로 이동
    PlayerController->ClientTravel(MapName, TRAVEL_Absolute);
}
*/