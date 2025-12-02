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

    // 블루프린트 + C++ 둘 다 구현 가능
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void Interact(class ACharacter* ByWho);
};
