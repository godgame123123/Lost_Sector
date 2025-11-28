// Fill out your copyright notice in the Description page of Project Settings.

#include "UServerBrowserWidget.h"
#include "Components/PanelWidget.h"
#include "Components/Button.h"
#include "MyGameInstance.h"
#include "UServerBrowserItemWidget.h"

UServerBrowserWidget::UServerBrowserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 서버 아이템 위젯 클래스 경로 설정
	static ConstructorHelpers::FClassFinder<UUserWidget> ServerItemClassAsset(TEXT("/Game/Team_Folder/LHJ/UI/WB_ServerBrowser_Item"));
	if (ServerItemClassAsset.Succeeded())
	{
		ServerItemClass = ServerItemClassAsset.Class;
	}
}

bool UServerBrowserWidget::Initialize()
{
	bool bSuccess = Super::Initialize();
	if (!bSuccess) return false;

	// 버튼 이벤트 바인딩
	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnBackClicked);
	}

	if (Btn_Refresh)
	{
		Btn_Refresh->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnRefreshClicked);
	}

	if (Btn_Join)
	{
		Btn_Join->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnJoinClicked);
	}

	// GameInstance 가져오기
	if (UWorld* World = GetWorld())
	{
		GameInstance = Cast<UMyGameInstance>(World->GetGameInstance());
	}

	return true;
}

void UServerBrowserWidget::SetOwningGameInstance(UMyGameInstance* InGameInstance)
{
	GameInstance = InGameInstance;
}

void UServerBrowserWidget::SetServerList(TArray<FServerData> InServerData)
{
	if (!ServerListPanel) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 기존 아이템 제거
	ServerListPanel->ClearChildren();

	// 새 서버 아이템 생성
	uint32 Index = 0;
	for (const FServerData& ServerData : InServerData)
	{
		if (!ServerItemClass) continue;

		UServerBrowserItemWidget* ServerItem = CreateWidget<UServerBrowserItemWidget>(World, ServerItemClass);
		if (!ServerItem) continue;

		// 서버 데이터 설정
		ServerItem->Setup(this, Index, ServerData);
		
		// 패널에 추가
		ServerListPanel->AddChild(ServerItem);
		
		Index++;
	}
}

void UServerBrowserWidget::SetSelectedIndex(uint32 InIndex)
{
	SelectedIndex = InIndex;

	// 모든 아이템의 선택 상태 업데이트
	if (!ServerListPanel) return;

	for (int32 i = 0; i < ServerListPanel->GetChildrenCount(); ++i)
	{
		if (UServerBrowserItemWidget* Item = Cast<UServerBrowserItemWidget>(ServerListPanel->GetChildAt(i)))
		{
			Item->SetSelected(SelectedIndex.IsSet() && SelectedIndex.GetValue() == i);
		}
	}
}

void UServerBrowserWidget::JoinSelectedServer()
{
	if (SelectedIndex.IsSet() && GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Joining server at index %d"), SelectedIndex.GetValue());
		GameInstance->Join(SelectedIndex.GetValue());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No server selected or GameInstance is null"));
	}
}

void UServerBrowserWidget::RefreshServerList() 
{
	if (GameInstance)
	{
		GameInstance->RefreshServerList();
	}
}

void UServerBrowserWidget::OnBackClicked()
{
	// 델리게이트 브로드캐스트
	OnBackClickedDelegate.Broadcast();
}

void UServerBrowserWidget::OnRefreshClicked()
{
	RefreshServerList();
}

void UServerBrowserWidget::OnJoinClicked()
{
	JoinSelectedServer();
}

