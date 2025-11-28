// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyInterface.h"
#include "UServerBrowserWidget.generated.h"

class UPanelWidget;
class UButton;
class UMyGameInstance;

/**
 * 서버 브라우저 위젯 (WB_ServerBrowser)
 */
UCLASS()
class LOSTSECTOR_API UServerBrowserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UServerBrowserWidget(const FObjectInitializer& ObjectInitializer);

	// MyGameInstance를 통해 서버 목록을 받기 위한 설정
	void SetOwningGameInstance(UMyGameInstance* InGameInstance);

protected:
	virtual bool Initialize() override;

public:
	// 서버 목록 설정
	UFUNCTION()
	void SetServerList(TArray<struct FServerData> InServerData);

	// 선택된 서버 인덱스 설정
	void SetSelectedIndex(uint32 InIndex);

	// 서버 조인
	UFUNCTION()
	void JoinSelectedServer();

	// 서버 목록 새로고침
	UFUNCTION()
	void RefreshServerList();

private:
	// UI 요소들 (OptionalWidget = true로 설정하여 블루프린트에 없어도 에러가 나지 않도록 함)
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UPanelWidget* ServerListPanel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UButton* Btn_Back;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UButton* Btn_Refresh;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UButton* Btn_Join;

	// 서버 아이템 위젯 클래스
	UPROPERTY()
	TSubclassOf<UUserWidget> ServerItemClass;

	// 선택된 서버 인덱스
	TOptional<uint32> SelectedIndex;

	// GameInstance 참조
	UPROPERTY()
	UMyGameInstance* GameInstance;

	// 버튼 이벤트 핸들러
	UFUNCTION()
	void OnBackClicked();

	UFUNCTION()
	void OnRefreshClicked();

	UFUNCTION()
	void OnJoinClicked();

public:
	// 백 버튼 델리게이트 (부모 위젯에서 사용)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackClicked);
	UPROPERTY(BlueprintAssignable)
	FOnBackClicked OnBackClickedDelegate;
};

