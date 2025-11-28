// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyGameInstance.h"
#include "UServerBrowserItemWidget.generated.h"

class UTextBlock;
class UButton;
class UServerBrowserWidget;
class UImage;
class UBorder;
class UWidget;

/**
 * 서버 브라우저 아이템 위젯 (WB_ServerBrowser_Item)
 */
UCLASS()
class LOSTSECTOR_API UServerBrowserItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 서버 데이터로 초기화
	void Setup(UServerBrowserWidget* InParent, uint32 InIndex, const FServerData& InServerData);

	// 선택 상태 설정
	void SetSelected(bool bInSelected);

	// 서버 데이터 업데이트 (플레이어 수 변경 등)
	void UpdateServerData(const FServerData& InServerData);

protected:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	virtual bool Initialize() override;

	// UI 요소들 (OptionalWidget = true로 설정하여 블루프린트에 없어도 에러가 나지 않도록 함)
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Text_ServerName;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Text_HostName;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Text_PlayerCount;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Text_Ping;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UButton* Btn_Select;

	// 선택 상태 표시용 UI (선택적)
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UBorder* Border_Selected;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UImage* Image_Selected;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UWidget* Widget_SelectedIndicator;

	// 호버 효과용 (선택적)
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UBorder* Border_Hover;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UImage* Image_Hover;

	// 부모 위젯과 인덱스
	UPROPERTY()
	UServerBrowserWidget* ParentWidget;

	uint32 ServerIndex;

	// 현재 서버 데이터
	FServerData CurrentServerData;

	// 버튼 이벤트 핸들러
	UFUNCTION()
	void OnItemClicked();

	// 선택 상태 업데이트 (시각적 피드백)
	void UpdateSelectionVisuals();

	// 호버 상태 업데이트
	void UpdateHoverVisuals(bool bInHovered);

	// 선택 상태
	bool bIsSelected = false;
	bool bIsHovered = false;
};

