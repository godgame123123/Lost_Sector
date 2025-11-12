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
    
    //virtual void PostLogin(APlayerController* NewPlayer) override;
    
   // virtual void Logout(AController* Exiting) override;

    // 필드맵으로 전환
  //  UFUNCTION(BlueprintCallable, Category = "Lobby")
   // void TransitionToFieldMap(APlayerController* PlayerController, const FString& MapName);
};