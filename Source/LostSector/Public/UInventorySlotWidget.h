#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "UInventorySlotWidget.generated.h"

class UDraggableItemWidget;

// 클래스 이름의 'U'를 하나 줄여서 파일명과 일치시킴
UCLASS()
class LOSTSECTOR_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	UDraggableItemWidget* CurrentItem;

protected:
	UPROPERTY(meta = (BindWidget))
	class UBorder* ItemContainer;

public:
	void PlaceItem(UDraggableItemWidget* NewItem);
};