// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerRow.generated.h"


UCLASS(BlueprintType, Blueprintable)
class UServerRow : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Server Row")
	void SetUp(class UMainMenu* InParent, int32 InIndex);

	// 블루프린트에서도 사용할 수 있도록 이벤트 노출
	UFUNCTION(BlueprintImplementableEvent, Category = "Server Row")
	void OnServerRowClicked();
	
	// 선택 상태 업데이트 (시각적 피드백용)
	UFUNCTION(BlueprintCallable, Category = "Server Row")
	void UpdateSelectionState(bool bIsSelected);

private:
	UFUNCTION()
	void OnClicked();

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Server Row")
	class UTextBlock* ServerName;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Server Row")
	class UTextBlock* HostUser;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Server Row")
	class UTextBlock* ConnectionFraction;

	UPROPERTY(BlueprintReadWrite, Category = "Server Row")
	bool bSelected;

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* RowButton;

	UPROPERTY()
	class UMainMenu* Parent;

	UPROPERTY()
	int32 SelfIndex;
};
