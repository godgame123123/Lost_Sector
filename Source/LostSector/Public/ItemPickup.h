#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemTypes.h"
#include "Interactable.h"
#include "ItemPickup.generated.h"

UCLASS()
class LOSTSECTOR_API AItemPickup : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AItemPickup();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Stack, Category = "Item")
    FItemStack Stack;

    UPROPERTY(EditAnywhere)
    float MaxUseDistance = 220.f;

protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* StaticMeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class USkeletalMeshComponent* SkeletalMeshComp;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnRep_Stack();

    void ApplyVisualFromData();

public:

    // 🔥 위치 튀는 버그 해결
    // Unreal Engine 5.3에서는 PostEditMove와 PostEditChangeProperty가 AActor의 멤버가 아니므로 제거
    // 대신 OnConstruction에서 처리합니다.
    
    virtual void Interact(class ACharacter* ByWho) override;
};
