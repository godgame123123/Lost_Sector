// Fill out your copyright notice in the Description page of Project Settings.


#include "UCreateGameWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/CheckBox.h"
#include "Kismet/GameplayStatics.h"

bool UUCreateGameWidget::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;
	if (CreateGameButton)
	{
		CreateGameButton->OnClicked.AddDynamic(this, &UUCreateGameWidget::HostGame);
	}
	return true;
}

void UUCreateGameWidget::HostGame()
{
	FString PlayerCountString = "4";
	if (PlayerAmountInput)
	{
		PlayerCountString = PlayerAmountInput->GetText().ToString();


	}

	int32 MaxPlayers = FCString::Atoi(*PlayerCountString);
	bool bIsLan = LanCheckBox ? LanCheckBox->IsChecked() : false;
		UE_LOG(LogTemp, Warning, TEXT("Host Game Request: Max Players = %d, Is LAN = %s"),
			MaxPlayers, bIsLan ? TEXT("True") : TEXT("False"));

}

		