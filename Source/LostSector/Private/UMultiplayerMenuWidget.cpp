// Fill out your copyright notice in the Description page of Project Settings.

#include "UMultiplayerMenuWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "UCreateGameWidget.h"
#include "UServerBrowserWidget.h"
#include "MyGameInstance.h"

UMultiplayerMenuWidget::UMultiplayerMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UMultiplayerMenuWidget::Initialize()
{
	bool bSuccess = Super::Initialize();
	if (!bSuccess) return false;

	// 버튼 이벤트 바인딩
	if (Btn_ShowHost)
	{
		Btn_ShowHost->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::OnShowHostClicked);
	}

	if (Btn_ShowFind)
	{
		Btn_ShowFind->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::OnShowFindClicked);
	}

	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::OnBackClicked);
	}

	// CreateGame 위젯의 백 버튼 이벤트 연결
	if (WB_CreateGame)
	{
		if (UUCreateGameWidget* CreateGameWidget = Cast<UUCreateGameWidget>(WB_CreateGame))
		{
			// CreateGame 위젯에 MyInterface 설정
			// TScriptInterface에서 IMyInterface*로 변환
			if (OwningInstance.GetInterface())
			{
				CreateGameWidget->SetOwningInstance(OwningInstance.GetInterface());
			}

			// 백 버튼 델리게이트 연결
			CreateGameWidget->OnBackClickedDelegate.AddDynamic(this, &UMultiplayerMenuWidget::OnCreateGameBack);
		}
	}

	// ServerBrowser 위젯의 백 버튼 이벤트 연결
	if (WB_ServerBrowser)
	{
		if (UServerBrowserWidget* ServerBrowserWidget = Cast<UServerBrowserWidget>(WB_ServerBrowser))
		{
			// ServerBrowser 위젯에 GameInstance 설정
			if (UWorld* World = GetWorld())
			{
				if (UMyGameInstance* GameInstance = Cast<UMyGameInstance>(World->GetGameInstance()))
				{
					ServerBrowserWidget->SetOwningGameInstance(GameInstance);
				}
			}

			// 백 버튼 델리게이트 연결
			ServerBrowserWidget->OnBackClickedDelegate.AddDynamic(this, &UMultiplayerMenuWidget::OnServerBrowserBack);
		}
	}

	return true;
}

void UMultiplayerMenuWidget::OnShowHostClicked()
{
	if (Switcher_Menu && WB_CreateGame)
	{
		Switcher_Menu->SetActiveWidget(WB_CreateGame);
	}
}

void UMultiplayerMenuWidget::OnShowFindClicked()
{
	if (Switcher_Menu && WB_ServerBrowser)
	{
		Switcher_Menu->SetActiveWidget(WB_ServerBrowser);
		
		// 서버 목록 새로고침
		if (OwningInstance)
		{
			OwningInstance->RefreshServerList();
		}
	}
}

void UMultiplayerMenuWidget::OnBackClicked()
{
	// 메인 메뉴로 돌아가기
	Shutdown();
	
	// UMyGameInstance로 캐스팅하여 LoadMainMenu 호출
	if (UWorld* World = GetWorld())
	{
		if (UMyGameInstance* GameInstance = Cast<UMyGameInstance>(World->GetGameInstance()))
		{
			GameInstance->LoadMainMenu();
		}
	}
}

void UMultiplayerMenuWidget::OnCreateGameBack()
{
	// CreateGame에서 백 버튼을 눌렀을 때 메인 화면으로
	if (Switcher_Menu && WB_CreateGame)
	{
		// 첫 번째 위젯(메인 화면)으로 돌아가기
		Switcher_Menu->SetActiveWidgetIndex(0);
	}
}

void UMultiplayerMenuWidget::OnServerBrowserBack()
{
	// ServerBrowser에서 백 버튼을 눌렀을 때 메인 화면으로
	if (Switcher_Menu && WB_ServerBrowser)
	{
		// 첫 번째 위젯(메인 화면)으로 돌아가기
		Switcher_Menu->SetActiveWidgetIndex(0);
	}
}

UServerBrowserWidget* UMultiplayerMenuWidget::GetServerBrowserWidget() const
{
	return Cast<UServerBrowserWidget>(WB_ServerBrowser);
}

