#include "UDraggableItemWidget.h" 
#include "UItemDragDropOperation.h"
#include "UInventorySlotWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"


FReply UDraggableItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 좌클릭인 경우에만 드래그 감지를 시작합니다.
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

void UDraggableItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	// 드래그 오퍼레이션 객체 생성
	UItemDragDropOperation* DragOp = NewObject<UItemDragDropOperation>();

	if (DragOp)
	{
		DragOp->DraggedItem = this;
		DragOp->DefaultDragVisual = this;
		DragOp->Pivot = EDragPivot::CenterCenter;
		DragOp->SourceSlot = this->GetTypedOuter<UInventorySlotWidget>();
		

		OutOperation = DragOp;
	}
}