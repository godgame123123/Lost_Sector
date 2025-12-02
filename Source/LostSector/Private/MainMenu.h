// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuBase.h"
#include "Framework/Text/TextLayout.h"
#include "MainMenu.generated.h"
UCLASS(BlueprintType, Blueprintable)
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
	UFUNCTION(BlueprintCallable, Category = "Main Menu")
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

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void SetSelectedIndex(int32 InIndex);
private:
	UPROPERTY()
	bool bHasSelectedIndex = false;
	
	UPROPERTY()
	int32 SelectedIndex = -1;

	// 서버 이름 저장 변수
	UPROPERTY()
	FString CachedServerName;

	// ServerHostName 텍스트 변경 이벤트 핸들러
	UFUNCTION()
	void OnServerHostNameTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION()
	void OnServerHostNameTextChanged(const FText& Text);

	TSubclassOf<UUserWidget> ServerRowClass;

};
