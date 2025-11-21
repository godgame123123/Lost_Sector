#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LOSTSECTOR_API UInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // 생성자 추가 
    UInteractionComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float Range = 250.f;

    // 클라 입력에서 호출
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void Use();

protected:
    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable)
    void Server_Use(const FVector_NetQuantize& EyeLoc, const FRotator& EyeRot);
};
