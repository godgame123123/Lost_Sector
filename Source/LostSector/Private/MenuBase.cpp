// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuBase.h"

void UMenuBase::SetOwningInstance(IMyInterface* InInstance)
{
	OwningInstance = InInstance;
}

void UMenuBase::StartUp()
{
	AddToViewport(10);
	bIsFocusable = true;
	FInputModeUIOnly Inputmode;
	Inputmode.SetWidgetToFocus(TakeWidget());
	Inputmode.SetLockMouseToViewportBehavior(
		EMouseLockMode::DoNotLock);

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	PC->SetInputMode(Inputmode);
	PC->bShowMouseCursor = true;
}

void UMenuBase::Shutdown()
{
	RemoveFromViewport();
	bIsFocusable = false;
	
	FInputModeGameOnly Inputmode;
	
	UWorld* World = GetWorld();
	if (!World) return;
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	PC->SetInputMode(Inputmode);
	PC->bShowMouseCursor = false;
}