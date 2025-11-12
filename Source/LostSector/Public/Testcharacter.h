#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InventoryComponent.h"
#include "InteractionComponent.h"
#include "TestCharacter.generated.h"

UCLASS()
class LOSTSECTOR_API ATestCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ATestCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UInventoryComponent* Inventory;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UInteractionComponent* Interaction;

protected:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    void Use();
};
