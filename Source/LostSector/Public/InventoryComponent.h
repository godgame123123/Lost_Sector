#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemTypes.h"
#include "CharacterDataStructs.h"
#include "InventoryComponent.generated.h"

class AItemPickup;

// 인벤토리가 바뀔 때 위젯에서 바인딩해서 쓰는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LOSTSECTOR_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    // 기본 슬롯 개수 (배낭 없이)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 BaseSlotCount = 30;

    // 배낭으로 확장 가능한 최대 슬롯 수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 MaxExpandedSlots = 10;

    // 현재 확장된 슬롯 수 (배낭 아이템으로 추가된 슬롯)
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
    int32 ExpandedSlotCount = 0;

    // 총 무게 제한
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    float WeightLimit = 100.f;

    // UI에서 바인딩하는 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    // 슬롯 배열 (RepNotify)
    UPROPERTY(ReplicatedUsing = OnRep_Slots, BlueprintReadOnly, Category = "Inventory")
    TArray<FItemStack> Slots;

    // 창고 슬롯 배열 (30개)
    UPROPERTY(ReplicatedUsing = OnRep_StorageSlots, BlueprintReadOnly, Category = "Inventory|Storage")
    TArray<FItemStack> StorageSlots;

    // ---------- 공개 함수들 ----------

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitSlots();

    // 배낭 아이템으로 인벤토리 확장
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool ExpandInventoryWithBag(int32 AdditionalSlots);

    // 배낭 제거 시 인벤토리 축소
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool ShrinkInventory(int32 SlotsToRemove);

    // 현재 총 슬롯 수 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    int32 GetTotalSlotCount() const { return BaseSlotCount + ExpandedSlotCount; }

    // 창고 초기화
    UFUNCTION(BlueprintCallable, Category = "Inventory|Storage")
    void InitStorageSlots();

    // 창고에 아이템 추가
    UFUNCTION(BlueprintCallable, Category = "Inventory|Storage")
    bool TryAddToStorage(const FItemStack& InStack, int32& OutAdded);

    // 창고에서 아이템 제거
    UFUNCTION(BlueprintCallable, Category = "Inventory|Storage")
    bool RemoveFromStorage(int32 Index, int32 Count);

    // 창고 아이템 이동
    UFUNCTION(BlueprintCallable, Category = "Inventory|Storage")
    bool TryMoveStorage(int32 FromIdx, int32 ToIdx);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    float GetTotalWeight() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddStack(const FItemStack& InStack, int32& OutAdded);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryMove(int32 FromIdx, int32 ToIdx);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TrySplit(int32 FromIdx, int32 NumToSplit, int32 ToIdx);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveAt(int32 Index, int32 Count);

    // 아이템 사용 (소비 가능한 아이템만)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(int32 Index);

    // 특정 아이템의 총 수량 가져오기 (ItemId로 검색)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    int32 GetItemCountByItemId(FName ItemId) const;

    // 특정 아이템의 총 수량 가져오기 (ItemDataBase로 검색)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    int32 GetItemCountByItemData(UItemDataBase* ItemData) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 RemoveItemByItemData(UItemDataBase* ItemData, int32 Count);

    // 총알 소비 (발사 시 사용)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool ConsumeAmmo(UItemDataBase* AmmoItemData, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TransferFrom(UInventoryComponent* From, int32 FromIdx, int32 Count, int32& OutMoved);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TransferAllFrom(UInventoryComponent* From, int32& OutTotalMoved);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool DropAt(int32 FromIdx, int32 Count, const FTransform& WorldTransform, TSubclassOf<AItemPickup> PickupClass);

    // 수동 저장 (블루프린트에서 호출용)
    UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
    void ManualSave();

    // ItemId로 Item 포인터 복원 (JSON 로드 후 사용)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RestoreItemPointers();

    // 창고 ItemId로 Item 포인터 복원 (JSON 로드 후 사용)
    UFUNCTION(BlueprintCallable, Category = "Inventory|Storage")
    void RestoreStorageItemPointers();

    // UI 업데이트 브로드캐스트 (JSON 로드 후 사용)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void BroadcastUpdated();

    // 디버깅: 인벤토리 상태 출력
    UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
    void DebugPrintInventory() const;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // RepNotify
    UFUNCTION()
    void OnRep_Slots();

    UFUNCTION()
    void OnRep_StorageSlots();

    // ---------- 서버 RPC ----------

    UFUNCTION(Server, Reliable)
    void Server_TryAddStack(const FItemStack& InStack);

    UFUNCTION(Server, Reliable)
    void Server_TryMove(int32 FromIdx, int32 ToIdx);

    UFUNCTION(Server, Reliable)
    void Server_TrySplit(int32 FromIdx, int32 NumToSplit, int32 ToIdx);

    UFUNCTION(Server, Reliable)
    void Server_TransferFrom(UInventoryComponent* From, int32 FromIdx, int32 Count);

    UFUNCTION(Server, Reliable)
    void Server_TransferAllFrom(UInventoryComponent* From);

    UFUNCTION(Server, Reliable)
    void Server_DropAt(int32 FromIdx, int32 Count, const FTransform& Xform, TSubclassOf<AItemPickup> PickupClass);

    UFUNCTION(Server, Reliable)
    void Server_UseItem(int32 Index);

private:
    bool CanAddWeight(float AddW) const;
    FORCEINLINE bool ValidIndex(int32 I) const { return Slots.IsValidIndex(I); }
    FORCEINLINE bool ValidStorageIndex(int32 I) const { return StorageSlots.IsValidIndex(I); }

    // 디바운스 저장 관련
    FTimerHandle SaveDebounceTimer;

    UPROPERTY(EditAnywhere, Category = "Inventory|Save", meta = (ClampMin = "0.5", ClampMax = "10.0"))
    float SaveDebounceDelay = 2.0f; // 2초 후 저장

    void ScheduleSave();
    void SaveInventoryToServer();
};
