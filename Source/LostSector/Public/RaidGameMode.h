#pragma once

#include "CoreMinimal.h"
#include "LostSectorGameMode.h"
#include "RaidGameMode.generated.h"

UCLASS()
class LOSTSECTOR_API ARaidGameMode : public ALostSectorGameMode
{
	GENERATED_BODY()

public:
	ARaidGameMode();

protected:
	virtual void BeginPlay() override;

	// 게임맵에서는 자동 게임 시작 기능 비활성화
	virtual void CheckAndStartGame() override;
};
