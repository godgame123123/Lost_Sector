#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemTypes.h"
#include "Interactable.h"
#include "ItemPickup.generated.h"

UCLASS(Blueprintable)
class LOSTSECTOR_API AItemPickup : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    AItemPickup();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Stack, Category = "Item")
    FItemStack Stack;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    float MaxUseDistance = 220.f;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* StaticMeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkeletalMeshComponent* SkeletalMeshComp;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void ApplyVisualFromData();

    UFUNCTION()
    void OnRep_Stack();

    virtual void OnConstruction(const FTransform& Transform) override;

public:
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

    virtual void Interact(class ACharacter* ByWho) override;
};
