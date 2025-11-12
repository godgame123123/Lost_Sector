#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(BlueprintType)
class LOSTSECTOR_API UInteractable : public UInterface
{
    GENERATED_BODY()
};

class LOSTSECTOR_API IInteractable
{
    GENERATED_BODY()
public:
    virtual void Interact(class ACharacter* ByWho) = 0;
};
