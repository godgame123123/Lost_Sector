#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LOSTSECTOR_API UInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) float Range = 250.f;

    UFUNCTION(BlueprintCallable) void Use(); // 클라 입력 호출

protected:
    virtual void BeginPlay() override;



    UFUNCTION(Server, Reliable)
    void Server_Use(const FVector_NetQuantize& EyeLoc, const FRotator& EyeRot);
};
