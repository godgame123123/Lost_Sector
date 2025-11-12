// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "AUIControllerBase.generated.h"

/**
 * 
 */
UCLASS()
class LOSTSECTOR_API AAUIControllerBase : public APlayerController
{
	GENERATED_BODY()
public:
	AAUIControllerBase();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Widgets")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Widgets")
	TSubclassOf<UUserWidget> InventoryWidgetClass;
private:
	UPROPERTY()
	UUserWidget* HUDWidgetInstance;

	UPROPERTY()
		UUserWidget* InventoryWidgetInstance;
public:
	UFUNCTION(BlueprintCallable, Category = "UI Management")
	void ShowHUD();

	UFUNCTION(BlueprintCallable, Category = "UI Management")
	void ToggleInventory();
		


	void ShowMainMenu();
};
