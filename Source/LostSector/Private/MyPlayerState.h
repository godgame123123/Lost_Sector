#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CharacterDataStructs.h"
#include "ItemTypes.h" // FItemStack 추가
#include "Net/UnrealNetwork.h"
#include "MyPlayerState.generated.h"

UCLASS()
class LOSTSECTOR_API AMyPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AMyPlayerState();
    
    UPROPERTY(Replicated, BlueprintReadOnly)
    FCharacterData PlayerData;
    
    // 창고 아이템 (리플리케이트)
    UPROPERTY(ReplicatedUsing=OnRep_StashItems, BlueprintReadOnly)
    TArray<FItemStack> StashItems;
    
    // 인벤토리 아이템 (리플리케이트)
    UPROPERTY(ReplicatedUsing=OnRep_InventoryItems, BlueprintReadOnly)
    TArray<FItemStack> InventoryItems;
    
    // 돈
    UPROPERTY(ReplicatedUsing=OnRep_Money, BlueprintReadOnly)
    int32 Money;
    
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    // 서버에서 데이터 로드
    UFUNCTION()
    void LoadPlayerDataFromServer();
    
    // 서버에서 데이터 저장
    UFUNCTION()
    void SavePlayerDataToServer();
    
    // 클라이언트 → 서버: 아이템 추가 요청
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_AddItemToStash(const FItemStack& ItemStack);
    
    // 클라이언트 → 서버: 아이템 제거 요청
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RemoveItemFromStash(int32 Index);
    
    UFUNCTION(BlueprintCallable)
    void UpgradeStat(EStatTypes Stat, int32 Amount);
    
protected:
    // 리플리케이션 콜백
    UFUNCTION()
    void OnRep_StashItems();
    
    UFUNCTION()
    void OnRep_InventoryItems();
    
    UFUNCTION()
    void OnRep_Money();
};
