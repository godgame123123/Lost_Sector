#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LOSTSECTOR_API UInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractionComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float Range = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float SphereRadius = 30.f;

    UFUNCTION(BlueprintCallable)
    void Use();

protected:
    virtual void BeginPlay() override;

    // 캐릭터 중심 상호작용용 서버 RPC
    UFUNCTION(Server, Reliable)
    void Server_Use(const FVector_NetQuantize& StartLoc, const FRotator& FacingRot);
};
