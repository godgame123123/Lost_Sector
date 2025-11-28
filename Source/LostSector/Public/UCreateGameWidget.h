// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UCreateGameWidget.generated.h"

class UButton;
class UEditableTextBox;
class UCheckBox;


/**
 * 
 */
UCLASS()
class LOSTSECTOR_API UUCreateGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HostGame();
protected:
	virtual bool Initialize() override;
private:
	UPROPERTY(meta=(BindWidget))
	UButton* CreateGameButton;

	UPROPERTY(meta=(BindWidget))
	UEditableTextBox* PlayerAmountInput;

	UPROPERTY(meta=(BindWidget))
	UCheckBox* LanCheckBox;

	
};
