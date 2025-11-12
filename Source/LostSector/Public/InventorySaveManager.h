#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemTypes.h"
#include "InventorySaveManager.generated.h"
/**
 * 
 */
UCLASS()
class LOSTSECTOR_API UInventorySaveManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// 서버: 플레이어 인벤토리 저장
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save", meta = (WorldContext = "WorldContextObject"))
	static bool SavePlayerInventory(UObject* WorldContextObject, const FString& PlayerID, 
								   const TArray<FItemStack>& InventorySlots, 
								   const TArray<FItemStack>& StorageSlots);

	// 서버: 플레이어 인벤토리 로드
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save", meta = (WorldContext = "WorldContextObject"))
	static bool LoadPlayerInventory(UObject* WorldContextObject, const FString& PlayerID, 
								   FPlayerInventorySaveData& OutData);

	// 자동 저장 활성화
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
	void EnableAutoSave(float IntervalSeconds = 300.0f);

	// 자동 저장 비활성화
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
	void DisableAutoSave();

private:
	FTimerHandle AutoSaveTimerHandle;
	static FString GetSaveFilePath(const FString& PlayerID);
	void AutoSaveAllPlayers();
};