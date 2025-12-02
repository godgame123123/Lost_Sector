#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "InventoryComponent.h"
#include "LootContainer.generated.h"

UCLASS()
class LOSTSECTOR_API ALootContainer : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UInventoryComponent* Inventory;
    UPROPERTY(Replicated) bool bLocked = false;
    UPROPERTY() TWeakObjectPtr<AController> LockedBy;
    UPROPERTY(EditAnywhere) float MaxUseDistance = 240.f;

    ALootContainer();

protected:
    // ✅ 헤더에선 선언만
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    virtual void Interact_Implementation(ACharacter* ByWho) override;


    // 컨테이너 인벤토리를 외부 목록으로 초기화하는 함수 (가상 함수로 선언)
    virtual void InitializeLoot(const TArray<FItemStack>& InventoryToStore);
private:
    bool TryLock(AController* By);
    void Unlock(AController* By);
};
