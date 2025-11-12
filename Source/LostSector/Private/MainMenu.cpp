// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "ServerRow.h"
#include "MyGameInstance.h"

UMainMenu::UMainMenu()
{
	ConstructorHelpers::FClassFinder<UUserWidget> ServerRowClass_Asset(TEXT("/Game/UI/WB_ServerRow"));
	if (ServerRowClass_Asset.Succeeded())
		ServerRowClass = ServerRowClass_Asset.Class;

}

bool UMainMenu::Initialize()
{
	bool bSuccess = Super::Initialize();
	if (!bSuccess) return false;

	if (HostButton != nullptr)
		HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);
	if (JoinButton != nullptr)
		JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

	if (CancelHostButton != nullptr)
		CancelHostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);
	if (CancelJoinButton != nullptr)
		CancelJoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

	if (ConfirmHostButton != nullptr)
		ConfirmHostButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);


	if (QuitButton != nullptr)
		QuitButton->OnClicked.AddDynamic(this, &UMainMenu::QuitGame);

	if (ConfirmJoinButton != nullptr)
		ConfirmJoinButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);


	return false;
}
void UMainMenu::QuitGame()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)return;

	PC->ConsoleCommand("quit");
}

void UMainMenu::JoinServer()
{

	if (SelectedIndex.IsSet() && OwningInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index is %d"), SelectedIndex.GetValue());
		OwningInstance->Join(SelectedIndex.GetValue());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index is not set"));
	}
}

void UMainMenu::SetSelectedIndex(uint32 InIndex)
{
	SelectedIndex = InIndex;
	for (int32 i=0; i < Serverlist->GetChildrenCount(); ++i)
	{
		auto serverRow = Cast<UServerRow>(Serverlist->GetChildAt(i));
		if (serverRow)
		{
			serverRow->bSelected = 
				(SelectedIndex.IsSet() && 
					SelectedIndex.GetValue() == i);
		}
	}
}

void UMainMenu::SetServerList(
	TArray<FServerData> InServerData)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (!Serverlist) return;
	Serverlist->ClearChildren();

	uint32 i = 0;
	for (const FServerData& ServerData : InServerData)
	{
		UServerRow* ServerRow = CreateWidget<UServerRow>(World, ServerRowClass);
		if (!ServerRow) return;

		ServerRow->ServerName->SetText(FText::FromString(ServerData.Name));
		ServerRow->HostUser->SetText(FText::FromString(ServerData.HostUserName));

		FString FractionText = FString::Printf(TEXT("%d/%d"),(ServerData.CurrentPlayers, ServerData.MaxPlayers));
		ServerRow->ConnectionFraction->SetText(FText::FromString(FractionText));
		ServerRow->SetUp(this,i++);
		Serverlist->AddChild(ServerRow);
	}
}

void UMainMenu::OpenMainMenu()
{
	if (!MenuSwitcher) return;
	if (!MainMenu) return;

	MenuSwitcher->SetActiveWidget(MainMenu);
}
void UMainMenu::OpenHostMenu()
{
	if (!MenuSwitcher) return;
	if (!HostMenu) return;
	MenuSwitcher->SetActiveWidget(HostMenu);
}
void UMainMenu::OpenJoinMenu()
{
	if (!MenuSwitcher) return;
	if (!JoinMenu) return;

	MenuSwitcher->SetActiveWidget(JoinMenu);

	if (OwningInstance)
		OwningInstance->RefreshServerList();
}

void UMainMenu::HostServer()
{
	FString ServerName= ServerHostName->Text.ToString();
	OwningInstance->Host(ServerName);
}

