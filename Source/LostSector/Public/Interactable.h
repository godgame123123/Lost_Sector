#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class ACharacter;

UINTERFACE(BlueprintType)
class LOSTSECTOR_API UInteractable : public UInterface
{
    GENERATED_BODY()
};

class LOSTSECTOR_API IInteractable
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void Interact(ACharacter* ByWho);
};
