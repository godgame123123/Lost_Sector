#pragma once
#include "CoreMinimal.h"
#include "ItemTypes.h" 
#include "ServerDataManager.generated.h"

UCLASS()
class LOSTSECTOR_API UServerDataManager : public UObject
{
	GENERATED_BODY()

public:
	// 서버에서만 실행
	static UServerDataManager* GetInstance(UWorld* World);
    
	// 플레이어 데이터 로드 (서버에서만)
	UFUNCTION()
	bool LoadPlayerData(const FString& PlayerID, FPlayerData& OutData);
    
	// 플레이어 데이터 저장 (서버에서만)
	UFUNCTION()
	bool SavePlayerData(const FString& PlayerID, const FPlayerData& Data);
    
private:
	FString GetPlayerDataPath(const FString& PlayerID);
    
	static UServerDataManager* Instance;
};

