// Fill out your copyright notice in the Description page of Project Settings.


#include "AUIControllerBase.h"

AAUIControllerBase::AAUIControllerBase()
{
}

void AAUIControllerBase::BeginPlay()
{
    Super::BeginPlay();

    ShowHUD();
}

void AAUIControllerBase::ShowHUD()
{
    if (HUDWidgetInstance)
    {
        return;
    }
    if (HUDWidgetClass)
    {
        HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
        if (HUDWidgetInstance)
        {
            HUDWidgetInstance->AddToViewport();
        }

    }
}
void AAUIControllerBase::ToggleInventory()
{
    if (InventoryWidgetInstance && InventoryWidgetInstance->IsInViewport())
    {
        InventoryWidgetInstance->RemoveFromParent();
        InventoryWidgetInstance = nullptr;

        SetInputMode(FInputModeGameOnly());
        bShowMouseCursor = false;
    }
    else
    {
        if(InventoryWidgetClass)
            if(!InventoryWidgetInstance)
            {
                InventoryWidgetInstance = CreateWidget<UUserWidget>(this, InventoryWidgetClass);
            }
        if (InventoryWidgetInstance)
        {
            InventoryWidgetInstance->AddToViewport();

            FInputModeUIOnly InputModeData;
            InputModeData.SetWidgetToFocus(InventoryWidgetInstance->TakeWidget());
            SetInputMode(InputModeData);
            bShowMouseCursor = true;
        }
    }
}

       
