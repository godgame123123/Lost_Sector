// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Interface.h"
#include "MyInterface.h"
#include "MenuBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class UMenuBase : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void SetOwningInstance(TScriptInterface<IMyInterface> InInstance);

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void StartUp();

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void Shutdown();

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Menu")
	TScriptInterface<IMyInterface> OwningInstance;

};
