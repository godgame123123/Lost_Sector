// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UDraggableItemWidget.generated.h"
/**
 * 
 */
UCLASS()
class LOSTSECTOR_API UUDraggableItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

    // 아이템 정보
    UPROPERTY(BlueprintReadWrite, Category = "Item")
    int32 ItemID;
};

};
