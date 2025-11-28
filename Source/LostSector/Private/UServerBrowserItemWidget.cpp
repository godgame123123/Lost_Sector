// Fill out your copyright notice in the Description page of Project Settings.

#include "UServerBrowserItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "UServerBrowserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Framework/Application/SlateApplication.h"

bool UServerBrowserItemWidget::Initialize()
{
	bool bSuccess = Super::Initialize();
	if (!bSuccess) return false;

	if (Btn_Select)
	{
		Btn_Select->OnClicked.AddDynamic(this, &UServerBrowserItemWidget::OnItemClicked);
	}

	return true;
}

void UServerBrowserItemWidget::Setup(UServerBrowserWidget* InParent, uint32 InIndex, const FServerData& InServerData)
{
	ParentWidget = InParent;
	ServerIndex = InIndex;
	CurrentServerData = InServerData;

	UpdateServerData(InServerData);
}

void UServerBrowserItemWidget::UpdateServerData(const FServerData& InServerData)
{
	CurrentServerData = InServerData;

	// 서버 정보 표시
	if (Text_ServerName)
	{
		FString ServerName = InServerData.Name.IsEmpty() ? TEXT("Unnamed Server") : InServerData.Name;
		Text_ServerName->SetText(FText::FromString(ServerName));
	}

	if (Text_HostName)
	{
		FString HostName = InServerData.HostUserName.IsEmpty() ? TEXT("Unknown") : InServerData.HostUserName;
		Text_HostName->SetText(FText::FromString(HostName));
	}

	if (Text_PlayerCount)
	{
		FString PlayerCountText = FString::Printf(TEXT("%d/%d"), InServerData.CurrentPlayers, InServerData.MaxPlayers);
		Text_PlayerCount->SetText(FText::FromString(PlayerCountText));
	}

	// Ping 표시 (서버 데이터에 Ping 정보가 있다면)
	// FServerData에 Ping 필드가 없으면 주석 처리
	// if (Text_Ping)
	// {
	//     FString PingText = FString::Printf(TEXT("%d ms"), InServerData.Ping);
	//     Text_Ping->SetText(FText::FromString(PingText));
	// }
}

void UServerBrowserItemWidget::SetSelected(bool bInSelected)
{
	bIsSelected = bInSelected;
	UpdateSelectionVisuals();
}

void UServerBrowserItemWidget::UpdateSelectionVisuals()
{
	// 선택 상태에 따른 시각적 피드백
	if (Border_Selected)
	{
		Border_Selected->SetVisibility(bIsSelected ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (Image_Selected)
	{
		Image_Selected->SetVisibility(bIsSelected ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (Widget_SelectedIndicator)
	{
		Widget_SelectedIndicator->SetVisibility(bIsSelected ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UServerBrowserItemWidget::UpdateHoverVisuals(bool bInHovered)
{
	bIsHovered = bInHovered;

	if (Border_Hover)
	{
		Border_Hover->SetVisibility(bInHovered ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (Image_Hover)
	{
		Image_Hover->SetVisibility(bInHovered ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UServerBrowserItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	UpdateHoverVisuals(true);
}

void UServerBrowserItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	UpdateHoverVisuals(false);
}

void UServerBrowserItemWidget::OnItemClicked()
{
	if (ParentWidget)
	{
		ParentWidget->SetSelectedIndex(ServerIndex);
	}
}

FReply UServerBrowserItemWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 더블 클릭 시 바로 서버 조인
	if (ParentWidget)
	{
		ParentWidget->SetSelectedIndex(ServerIndex);
		ParentWidget->JoinSelectedServer();
	}
	
	return FReply::Handled();
}

