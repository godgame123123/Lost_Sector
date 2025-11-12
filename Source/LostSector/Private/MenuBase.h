// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyInterface.h"
#include "MenuBase.generated.h"

/**
 * 
 */
UCLASS()
class UMenuBase : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetOwningInstance(IMyInterface* InInstance);

	void StartUp();
	void Shutdown();

protected:
	IMyInterface* OwningInstance;//게임인스턴스
};
