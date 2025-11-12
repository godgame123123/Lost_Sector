
#pragma once
#include "CoreMinimal.h"
#include "CharacterDataStructs.generated.h"

UENUM(BlueprintType)
enum EStatTypes : uint8
{
	Hp      UMETA(DisplayName = "HP"),
	Stamina UMETA(DisplayName = "Stamina"),
	Hungry  UMETA(DisplayName = "Hungry"),
	Weight  UMETA(DisplayName = "Weight"),
};

USTRUCT(BlueprintType)
struct FCharacterData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Hp = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Stamina = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 hungry = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 weight = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Abilities;
};
