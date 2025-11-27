#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UDraggableItemWidget.generated.h" // [수정] 파일명과 일치시킴

// [수정] 클래스 이름 U 하나 제거 (표준 준수)
UCLASS()
class LOSTSECTOR_API UDraggableItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 ItemID;
};