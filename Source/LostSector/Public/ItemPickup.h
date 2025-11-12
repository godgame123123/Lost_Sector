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

    // ✅ 스택이 바뀌면 클라에서도 외형 갱신하도록 ReplicatedUsing
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Stack)
    FItemStack Stack;

    UPROPERTY(EditAnywhere)
    float MaxUseDistance = 220.f;

protected:
    // ✅ 에디터에서 보이게 UPROPERTY로 선언
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* StaticMeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkeletalMeshComponent* SkeletalMeshComp;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ✅ DataAsset → 컴포넌트 반영
    void ApplyVisualFromData();

    // ✅ 클라에서 Stack 복제되면 호출
    UFUNCTION()
    void OnRep_Stack();

    // ✅ 에디터에서 값 바꾸자마자 반영 (레벨 배치 시)
    virtual void OnConstruction(const FTransform& Transform) override;

public:
    virtual void Interact(class ACharacter* ByWho) override;
};
