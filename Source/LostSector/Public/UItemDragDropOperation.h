#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "UItemDragDropOperation.generated.h"

// 전방 선언 (순환 참조 방지)
class UDraggableItemWidget;
class UInventorySlotWidget;

/**
 * 드래그 정보를 전달하는 객체입니다.
 */
UCLASS()
class LOSTSECTOR_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	// 드래그 중인 아이템 위젯의 포인터
	
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn), Category = "Drag Drop")
	UDraggableItemWidget* DraggedItem;
	// 아이템이 드래그되기 전의 원본 슬롯 위젯 포인터
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn), Category = "Drag Drop")
	UInventorySlotWidget* SourceSlot;
};