// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "UItemDragDropOperation.generated.h"
#include "UItemDragDropOperation.h"

UCLASS()
class LOSTSECTOR_API UItemDragDropOperation : public UDragDropOperation
{
    GENERATED_BODY()

public:
    // 드래그되는 아이템 위젯
    UPROPERTY()
    UDraggableItemWidget* DraggedItem;
};
