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

    // 슬롯 개수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 SlotCount = 30;

    // 총 무게 제한
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    float WeightLimit = 30.f;

    // UI에서 바인딩하는 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    // 슬롯 배열 (RepNotify)
    UPROPERTY(ReplicatedUsing = OnRep_Slots, BlueprintReadOnly, Category = "Inventory")
    TArray<FItemStack> Slots;

    // ---------- 공개 함수들 ----------

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitSlots();

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

private:
    bool CanAddWeight(float AddW) const;
    FORCEINLINE bool ValidIndex(int32 I) const { return Slots.IsValidIndex(I); }

    // 디바운스 저장 관련
    FTimerHandle SaveDebounceTimer;

    UPROPERTY(EditAnywhere, Category = "Inventory|Save", meta = (ClampMin = "0.5", ClampMax = "10.0"))
    float SaveDebounceDelay = 2.0f; // 2초 후 저장

    void ScheduleSave();
    void SaveInventoryToServer();
};
