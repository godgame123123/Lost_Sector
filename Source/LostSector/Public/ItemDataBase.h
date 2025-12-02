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

    // =====================
    // 기본 정보
    // =====================

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


    // =====================
    // 🔥 그리드 인벤토리 정보 추가 (타르코프 방식)
    // =====================

    // 아이템의 가로 칸 수 (기본 1칸)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Grid")
    int32 GridWidth = 1;

    // 아이템의 세로 칸 수 (기본 1칸)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Grid")
    int32 GridHeight = 1;


    // =====================
    // 월드에서 보이는 Mesh 설정
    // =====================

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


    // =====================
    // 경제 / 가치
    // =====================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
    int32 Value = 0;


    // =====================
    // 소비 아이템 효과
    // =====================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    float HealAmount = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    float StaminaAmount = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    float HungerAmount = 0.f;


    // =====================
    // 기능 함수
    // =====================

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
    bool IsConsumable() const
    {
        return Type == EItemType::Heal || Type == EItemType::Food;
    }
};
