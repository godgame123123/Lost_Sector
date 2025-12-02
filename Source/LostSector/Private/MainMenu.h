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
	virtual void NativeConstruct() override;
	virtual bool Initialize() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
private:


//MenuSwitcher
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;
#pragma region Main Menu
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;//���� �޴�����

	UPROPERTY(meta = (BindWidget))
	class UButton* ConfirmJoinButton;//�ش� �濡 �����ϴ°�


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


	UFUNCTION(BlueprintCallable)
	void OpenMainMenu();

	UFUNCTION(BlueprintCallable)
	void OpenHostMenu();

	UFUNCTION(BlueprintCallable)
	void OpenJoinMenu();

	UFUNCTION(BlueprintCallable)
	void HostServer();
	UFUNCTION(BlueprintCallable)
	void QuitGame();

	UFUNCTION(BlueprintCallable)
	void JoinServer();

	void SetSelectedIndex(uint32 InIndex);
private:
	TOptional<uint32> SelectedIndex;


	TSubclassOf<UUserWidget> ServerRowClass;

};
