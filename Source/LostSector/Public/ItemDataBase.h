#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataBase.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Weapon, Ammo, Heal, Food, Armor, Junk, Valuable, BossItem, Bag
};

UCLASS(BlueprintType)
class LOSTSECTOR_API UItemDataBase : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    UTexture2D* Icon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    float Weight = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 MaxStack = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType Type = EItemType::Junk;

    // ✅ 새로 추가해야 하는 부분
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<UStaticMesh> WorldStaticMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<USkeletalMesh> WorldSkeletalMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    FRotator WorldMeshRotation = FRotator::ZeroRotator;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    FVector  WorldMeshOffset = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (ClampMin = "0.001"))
    float    WorldMeshScale = 1.0f;

    // (가치 추가 필드도 같이 유지)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
    int32 Value = 0;
};
