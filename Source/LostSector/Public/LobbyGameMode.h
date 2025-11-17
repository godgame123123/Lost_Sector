#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

UCLASS()
class LOSTSECTOR_API ALobbyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ALobbyGameMode();

    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
    
    virtual void PostLogin(APlayerController* NewPlayer) override;
    
    virtual void Logout(AController* Exiting) override;

    // 필드맵으로 전환
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void TransitionToFieldMap(APlayerController* PlayerController, const FString& MapName);

protected:
    // 최소 플레이어 수 (게임 시작 가능한 최소 인원)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby Settings", meta = (ClampMin = "1", ClampMax = "24"))
    int32 MinPlayersToStart = 2;

    // 최대 플레이어 수
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby Settings", meta = (ClampMin = "1", ClampMax = "24"))
    int32 MaxPlayers = 3;

    // 2명일 때 게임 시작 전 대기 시간 (초) - 추가 인원이 없으면 이 시간 후 시작
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby Settings", meta = (ClampMin = "0", ClampMax = "600"))
    float StartDelayWith2Players = 180.0f; // 3분 = 180초

    // 3명일 때 게임 시작 전 대기 시간 (초) - 최대 인원이면 이 시간 후 시작
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby Settings", meta = (ClampMin = "0", ClampMax = "60"))
    float StartDelayWithMaxPlayers = 5.0f; // 5초

    // 게임 맵으로 이동
    UFUNCTION()
    void TravelToGameMap();

    // 인원 체크 및 게임 시작
    void CheckAndStartGame();

    // 테스트용 콘솔 명령어
    UFUNCTION(Exec)
    void TestLobbyStatus(); // 로비 상태 확인 (인원, 타이머 등)

private:
    FTimerHandle StartGameTimerHandle;
    bool bGameStarting = false; // 게임 시작 중 플래그 (중복 실행 방지)
};