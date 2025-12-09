// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemDataBase.h"
#include "ShopUtilityLibrary.generated.h"

/**
 * 
 */
UCLASS()
class LOSTSECTOR_API UShopUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,Category="Shop Sorting")
	static void SortItemsByPrice(UPARAM(ref) TArray<UItemDataBase*>& Items);
};
	

