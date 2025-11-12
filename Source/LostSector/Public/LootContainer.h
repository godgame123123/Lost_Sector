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
    virtual void Interact(class ACharacter* ByWho) override;

private:
    bool TryLock(AController* By);
    void Unlock(AController* By);
};
