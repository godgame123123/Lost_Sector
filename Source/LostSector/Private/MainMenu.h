// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuBase.h"
#include "MainMenu.generated.h"
UCLASS()
class UMainMenu : public UMenuBase
{
	GENERATED_BODY()
public:
	UMainMenu();
protected:
	virtual bool Initialize() override;
private:


//MenuSwitcher
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;
#pragma region Main Menu
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;//조인 메뉴오픈

	UPROPERTY(meta = (BindWidget))
	class UButton* ConfirmJoinButton;//해당 방에 조인하는거


	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* CancelJoinButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* CancelHostButton;
#pragma endregion Main Menu

	UPROPERTY(meta = (BindWidget))
	class UWidget* MainMenu;

	UPROPERTY(meta = (BindWidget))
	class UWidget* HostMenu;

	UPROPERTY(meta = (BindWidget))
	class UWidget* JoinMenu;

//HostMenu
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* ServerHostName;

	UPROPERTY(meta = (BindWidget))
	class UButton* ConfirmHostButton;
//HostMenu


	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* Serverlist;
public:
	UFUNCTION()
	void SetServerList(
		TArray<FServerData> InServerData);


	UFUNCTION()
	void OpenMainMenu();

	UFUNCTION()
	void OpenHostMenu();

	UFUNCTION()
	void OpenJoinMenu();

	UFUNCTION()
	void HostServer();
	UFUNCTION()
	void QuitGame();

	UFUNCTION()
	void JoinServer();

	void SetSelectedIndex(uint32 InIndex);
private:
	TOptional<uint32> SelectedIndex;


	TSubclassOf<UUserWidget> ServerRowClass;

};
