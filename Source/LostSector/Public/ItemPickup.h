#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemTypes.h"
#include "Interactable.h"
#include "ItemPickup.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class USceneComponent;

UCLASS()
class LOSTSECTOR_API AItemPickup : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AItemPickup();

    /** 아이템 스택 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Stack, Category = "Item")
    FItemStack Stack;

    UPROPERTY(EditAnywhere, Category = "Item")
    float MaxUseDistance = 220.f;

protected:
    /** ✅ Root (절대 위치 변경 금지) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    /** Static Mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StaticMeshComp;

    /** Skeletal Mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletalMeshComp;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_Stack();

    void ApplyVisualFromData();

public:
    // 🔥 UE5.3: PostEditMove 없음 → Property 변경만 처리
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

    // BlueprintNativeEvent
    virtual void Interact_Implementation(ACharacter* ByWho);
};
