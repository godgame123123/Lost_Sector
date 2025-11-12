#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemTypes.h"
#include "CharacterDataStructs.h"
#include "InventoryComponent.generated.h"   // ✅ 마지막 include

class AItemPickup;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LOSTSECTOR_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UInventoryComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 SlotCount = 30;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float WeightLimit = 30.f;

    // ✅ 반드시 UPROPERTY 이어야 DOREPLIFETIME 가능
    UPROPERTY(ReplicatedUsing = OnRep_Slots, BlueprintReadOnly)
    TArray<FItemStack> Slots;

    UFUNCTION(BlueprintCallable) void  InitSlots();
    UFUNCTION(BlueprintCallable) float GetTotalWeight() const;

    UFUNCTION(BlueprintCallable) bool TryAddStack(const FItemStack& InStack, int32& OutAdded);
    UFUNCTION(BlueprintCallable) bool TryMove(int32 FromIdx, int32 ToIdx);
    UFUNCTION(BlueprintCallable) bool TrySplit(int32 FromIdx, int32 NumToSplit, int32 ToIdx);
    UFUNCTION(BlueprintCallable) bool RemoveAt(int32 Index, int32 Count);

    UFUNCTION(BlueprintCallable) bool TransferFrom(UInventoryComponent* From, int32 FromIdx, int32 Count, int32& OutMoved);
    UFUNCTION(BlueprintCallable) bool TransferAllFrom(UInventoryComponent* From, int32& OutTotalMoved);

    UFUNCTION(BlueprintCallable) bool DropAt(int32 FromIdx, int32 Count, const FTransform& WorldTransform, TSubclassOf<AItemPickup> PickupClass);

    // 수동 저장 (블루프린트용)
    UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
    void ManualSave();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION() void OnRep_Slots();

    // 서버 RPC들
    UFUNCTION(Server, Reliable) void Server_TryAddStack(const FItemStack& InStack);
    UFUNCTION(Server, Reliable) void Server_TryMove(int32 FromIdx, int32 ToIdx);
    UFUNCTION(Server, Reliable) void Server_TrySplit(int32 FromIdx, int32 NumToSplit, int32 ToIdx);
    UFUNCTION(Server, Reliable) void Server_TransferFrom(UInventoryComponent* From, int32 FromIdx, int32 Count);
    UFUNCTION(Server, Reliable) void Server_TransferAllFrom(UInventoryComponent* From);
    UFUNCTION(Server, Reliable) void Server_DropAt(int32 FromIdx, int32 Count, const FTransform& Xform, TSubclassOf<AItemPickup> PickupClass);

private:
    bool CanAddWeight(float AddW) const;
    FORCEINLINE bool ValidIndex(int32 I) const { return Slots.IsValidIndex(I); }
    void BroadcastUpdated();

    // 디바운싱 저장 관련
    FTimerHandle SaveDebounceTimer;
    
    UPROPERTY(EditAnywhere, Category = "Inventory|Save", meta = (ClampMin = "0.5", ClampMax = "10.0"))
    float SaveDebounceDelay = 2.0f; // 2초 후 저장
    
    void ScheduleSave();
    void SaveInventoryToServer();
};
