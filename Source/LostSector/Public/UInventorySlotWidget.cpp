#include "UInventorySlotWidget.h"
#include "UDraggableItemWidget.h"
#include "UItemDragDropOperation.h"
#include "Components/BorderSlot.h"
#include "Types/SlateEnums.h" // 정렬 Enum 사용을 위해 추가 권장

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* ItemOp = Cast<UItemDragDropOperation>(InOperation);
	if (!ItemOp || !ItemOp->DraggedItem)
	{
		return false;
	}

	UDraggableItemWidget* DraggedWidget = ItemOp->DraggedItem;

	// [수정] 더 안전한 부모 슬롯 찾기 방식 (GetTypedOuter 사용)
	// 위젯 계층 구조를 타고 올라가서 가장 가까운 UInventorySlotWidget을 찾습니다.
	UInventorySlotWidget* OriginalSlot = ItemOp->SourceSlot;

	// A. 드래그된 아이템을 기존 부모에서 떼어냅니다.
	DraggedWidget->RemoveFromParent();

	// B. 스왑 로직 (현재 슬롯에 이미 아이템이 있는 경우)
	if (CurrentItem)
	{
		// 원래 아이템과 드래그된 아이템이 같다면(같은 슬롯에 드롭), 아무것도 안 하고 리턴하거나 그냥 진행해도 됩니다.
		if (CurrentItem == DraggedWidget) return true;

		CurrentItem->RemoveFromParent();

		// 원래 있던 슬롯으로 현재 아이템을 보냅니다.
		if (OriginalSlot)
		{
			OriginalSlot->PlaceItem(CurrentItem);
		}
		else
		{
			// 원래 슬롯을 못 찾았다면? (예: 인벤토리 밖에서 생성됨)
			// 아이템을 파괴하거나, 다시 인벤토리에 넣는 예외 처리가 필요할 수 있습니다.
			// 여기서는 일단 그냥 둡니다.
		}
	}

	// C. 드래그된 아이템을 이 슬롯에 배치합니다.
	this->PlaceItem(DraggedWidget);

	return true;
}

void UInventorySlotWidget::PlaceItem(UDraggableItemWidget* NewItem)
{
	if (!NewItem || !ItemContainer) return;

	CurrentItem = NewItem;

	// Border에 자식으로 추가하면 반환값으로 Slot 객체를 줍니다. 이를 활용하는 것이 더 확실합니다.
	UPanelSlot* NewSlot = ItemContainer->AddChild(NewItem);

	// 아이템 정렬 설정
	if (UBorderSlot* SlotProperties = Cast<UBorderSlot>(NewSlot))
	{
		SlotProperties->SetPadding(FMargin(0.f));
		// Enum 네임스페이스 명시 (컴파일 안전성 확보)
		SlotProperties->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		SlotProperties->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
	}

	NewItem->SetVisibility(ESlateVisibility::Visible);
}