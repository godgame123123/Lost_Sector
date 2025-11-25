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
    float Range = 350.f;   // 기본 350~500이 좋은 값

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float SphereRadius = 25.f;  // SphereTrace 반경(30이상도 가능)

    UFUNCTION(BlueprintCallable)
    void Use();   // 클라이언트가 호출하는 함수

protected:
    UFUNCTION(Server, Reliable)
    void Server_Use(const FVector_NetQuantize& EyeLoc, const FRotator& EyeRot);

    virtual void BeginPlay() override;
};
