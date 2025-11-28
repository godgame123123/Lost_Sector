// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuBase.h"
#include "UMultiplayerMenuWidget.generated.h"

class UWidgetSwitcher;
class UButton;
class UUserWidget;

/**
 * 멀티플레이어 메뉴 위젯 (WB_MultiplayerMenu)
 */
UCLASS()
class LOSTSECTOR_API UMultiplayerMenuWidget : public UMenuBase
{
	GENERATED_BODY()

public:
	UMultiplayerMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual bool Initialize() override;

	// WidgetSwitcher - CreateGame과 ServerBrowser를 전환
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "UI")
	UWidgetSwitcher* Switcher_Menu;

	// 서브 위젯들
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "UI")
	UUserWidget* WB_CreateGame;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "UI")
	UUserWidget* WB_ServerBrowser;

private:
	// 버튼들
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_ShowHost;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_ShowFind;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Back;

	// 버튼 이벤트 핸들러
	UFUNCTION()
	void OnShowHostClicked();

	UFUNCTION()
	void OnShowFindClicked();

	UFUNCTION()
	void OnBackClicked();

	// CreateGame 위젯의 백 버튼 이벤트를 받기 위한 델리게이트
	UFUNCTION()
	void OnCreateGameBack();

	// ServerBrowser 위젯의 백 버튼 이벤트를 받기 위한 델리게이트
	UFUNCTION()
	void OnServerBrowserBack();

public:
	// ServerBrowser 위젯에 접근하기 위한 함수
	class UServerBrowserWidget* GetServerBrowserWidget() const;
};

