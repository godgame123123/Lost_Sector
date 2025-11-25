#pragma once
#include "CoreMinimal.h"
#include "ItemDataBase.h"
#include "ItemTypes.generated.h"   // ← 항상 마지막 include

USTRUCT(BlueprintType)
struct FItemStack          // ← API 매크로 넣지 마세요
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UItemDataBase* Item = nullptr;

    // ✅ 네트워크 복제를 위해 ItemId 추가 (Item 포인터는 복제되지 않을 수 있음)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    // ✅ Item이 설정될 때 ItemId도 함께 업데이트
    void SetItem(UItemDataBase* InItem)
    {
        Item = InItem;
        ItemId = InItem ? InItem->ItemId : NAME_None;
    }

    bool IsValid() const { return Item != nullptr && Count > 0; }
    bool  IsSame(const FItemStack& O) const { return Item && Item == O.Item; }
    int32 FreeSpace()   const { return Item ? FMath::Max(0, Item->MaxStack - Count) : 0; }
    float TotalWeight() const { return Item ? Item->Weight * Count : 0.f; }

    int64 TotalValue()  const { return Item ? static_cast<int64>(Item->Value) * Count : 0; }
};

// ✅ 플레이어 데이터 구조체 추가
USTRUCT(BlueprintType)
struct FPlayerData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    TArray<FItemStack> StashItems;
    
    UPROPERTY(BlueprintReadWrite)
    TArray<FItemStack> InventoryItems;
    
    UPROPERTY(BlueprintReadWrite)
    int32 Money = 0;
    
    UPROPERTY(BlueprintReadWrite)
    int32 Level = 1;

};
USTRUCT(BlueprintType)
struct FPlayerInventorySaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString PlayerID;

    UPROPERTY(BlueprintReadWrite)
    FString PlayerName;

    UPROPERTY(BlueprintReadWrite)
    TArray<FItemStack> InventorySlots;

    UPROPERTY(BlueprintReadWrite)
    TArray<FItemStack> StorageSlots;

    UPROPERTY(BlueprintReadWrite)
    FDateTime LastSaveTime;
};

// 로컬 프로필 데이터
USTRUCT(BlueprintType)
struct FLocalPlayerProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString PlayerName;

    UPROPERTY(BlueprintReadWrite)
    FString UniquePlayerID;

    UPROPERTY(BlueprintReadWrite)
    FDateTime CreatedDate;

    UPROPERTY(BlueprintReadWrite)
    FDateTime LastPlayedDate;
};