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

    // ✔ 아이콘은 이것만 유지 (중복 제거)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    UTexture2D* Icon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    float Weight = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 MaxStack = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType Type = EItemType::Junk;

    // === Visual ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<UStaticMesh> WorldStaticMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<USkeletalMesh> WorldSkeletalMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    FRotator WorldMeshRotation = FRotator::ZeroRotator;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    FVector WorldMeshOffset = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (ClampMin = "0.001"))
    float WorldMeshScale = 1.0f;

    // === Economy ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
    int32 Value = 0;

    // === Item Effects (for consumables) ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    float HealAmount = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    float StaminaAmount = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    float HungerAmount = 0.f;

    // 아이템이 사용 가능한지 확인
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
    bool IsConsumable() const
    {
        return Type == EItemType::Heal || Type == EItemType::Food;
    }
};
