// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyInterface.h"
#include "UCreateGameWidget.generated.h"

class UButton;
class UEditableTextBox;
class UCheckBox;


/**
 * 게임 생성 위젯 (WB_CreateGame)
 */
UCLASS()
class LOSTSECTOR_API UUCreateGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UUCreateGameWidget(const FObjectInitializer& ObjectInitializer);

	// MyInterface 설정 (서버 호스팅을 위해)
	void SetOwningInstance(IMyInterface* InInstance);

	UFUNCTION()
	void HostGame();

	// 백 버튼 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackClicked);
	UPROPERTY(BlueprintAssignable)
	FOnBackClicked OnBackClickedDelegate;

protected:
	virtual bool Initialize() override;

private:
	// 위젯 참조 (수동 바인딩)
	UButton* CreateGameButton;
	UEditableTextBox* ServerNameInput;
	UEditableTextBox* PlayerAmountInput;
	UCheckBox* LanCheckBox;
	UButton* Btn_Back;

	// MyInterface 참조 (UPROPERTY 없이 사용 - 인터페이스는 UObject가 아니므로)
	IMyInterface* OwningInstance;

	// 버튼 이벤트 핸들러
	UFUNCTION()
	void OnBackClicked();
};
