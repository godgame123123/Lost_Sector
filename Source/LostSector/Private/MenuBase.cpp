// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuBase.h"

void UMenuBase::SetOwningInstance(TScriptInterface<IMyInterface> InInstance)
{
	OwningInstance = InInstance;
	
	// 디버깅: OwningInstance 설정 확인
	if (OwningInstance.GetInterface())
	{
		UE_LOG(LogTemp, Log, TEXT("[MenuBase] SetOwningInstance: OwningInstance 설정 완료 - 유효한 인터페이스"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MenuBase] SetOwningInstance: OwningInstance 설정 실패 - 인터페이스가 null입니다!"));
	}
}

void UMenuBase::StartUp()
{
	AddToViewport(10);
	
	// bIsFocusable은 생성 시에만 설정 가능하므로 제거
	// 위젯의 Details 패널에서 "Is Focusable"을 체크하세요
	
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
	RemoveFromParent();
	
	// bIsFocusable은 생성 시에만 설정 가능하므로 제거
	
	FInputModeGameOnly Inputmode;
	
	UWorld* World = GetWorld();
	if (!World) return;
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	PC->SetInputMode(Inputmode);
	PC->bShowMouseCursor = false;
}