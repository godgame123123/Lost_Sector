// Fill out your copyright notice in the Description page of Project Settings.


#include "UInventorySlotWidget.h"
#include "UDraggableItemWidget.h"
#include "UItemDragDropOperation.h"
#include <WinUser.h>

// UMG 오버라이드 시그니처에 맞게 UDragDropOperation* 타입을 사용합니다.
bool UUInventorySlotWidget::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation) // 1. 시그니처 수정 완료
{
    // 드롭된 Operation이 우리가 예상한 커스텀 타입인지 확인합니다.
    if (UUItemDragDropOperation* ItemOp = Cast<UUItemDragDropOperation>(InOperation))
    {
        // ItemOp 안에 저장된, 실제로 드래그된 아이템 위젯을 가져옵니다.
        // ItemOp::DraggedItem이 UDraggableItemWidget* 타입이라고 가정합니다.
        UUDraggableItemWidget* DraggedItemWidget = ItemOp->DraggedItem; // 2. ItemOp 멤버 사용

        if (!DraggedItemWidget)
        {
            return false;
        }

        // 드래그된 위젯이 원래 속해있던 부모 슬롯을 찾습니다.
        // GetParent()는 UWidget의 함수이므로, DraggedItemWidget을 UWidget*으로 간주해야 합니다.
        UUInventorySlotWidget* OriginalSlot = Cast<UUInventorySlotWidget>(DraggedItemWidget->GetParent());

        if (OriginalSlot)
        {
            // --- 아이템 스왑 로직 시작 ---

            // 1. 드래그된 아이템을 **현재 슬롯**에서 제거합니다.
            DraggedItemWidget->RemoveFromParent();

            // 2. 현재 슬롯의 아이템을 **원래 슬롯**으로 이동합니다.
            OriginalSlot->CurrentItem = CurrentItem;
            if (CurrentItem) // 현재 슬롯에 아이템이 있다면
            {
                CurrentItem->RemoveFromParent(); // 현재 슬롯에서 제거
                OriginalSlot->AddChild(CurrentItem); // 원래 슬롯에 추가
            }

            // 3. 드래그된 아이템을 **현재 슬롯**에 추가하고 상태를 업데이트합니다.
            CurrentItem = DraggedItemWidget;
            AddChild(DraggedItemWidget);

            return true;
        }
    }

    return false;
}
