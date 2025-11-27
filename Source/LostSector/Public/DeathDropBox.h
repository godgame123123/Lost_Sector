// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LootContainer.h"
#include "DeathDropBox.generated.h"

/**
 * 
 */
UCLASS()
class LOSTSECTOR_API ADeathDropBox : public ALootContainer
{
	GENERATED_BODY()
	
public:
	ADeathDropBox();

	// 부모 클래스(ALootContainer)에서 상속받은 함수를 명시적으로 선언합니다.
	// 이 함수의 구현은 ALootContainer.cpp에 이미 있습니다.
	void InitializeLoot(const TArray<FItemStack>& InventoryToStore) override;
};
