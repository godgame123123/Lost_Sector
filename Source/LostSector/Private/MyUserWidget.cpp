// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "Components/Button.h" 
#include "Kismet/KismetSystemLibrary.h" // QuitGame �Լ� ���뵵
#include "Kismet/GameplayStatics.h" // OpenLevel �Լ� ���뵵

void UMyUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (StartGame)
	{
        StartGame->OnClicked.AddDynamic(this, &UMyUserWidget::OnStartGameClicked);
    }
    if (Options)
    {
        Options->OnClicked.AddDynamic(this, &UMyUserWidget::OnOptionClicked);
    }
    if (QuitGame)
    {
        QuitGame->OnClicked.AddDynamic(this, &UMyUserWidget::OnQuitGameClicked);
    }
	

	
}

void UMyUserWidget::OnStartGameClicked()
{
    UE_LOG(LogTemp,Warning, TEXT("Start Game Button Clicked! Starting Level..."));
    UGameplayStatics:: OpenLevel(GetWorld(), TEXT("YourGameLevelName"));
}

void UMyUserWidget::OnOptionClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Options Button Clicked! Showing Options UI..."));
}

void UMyUserWidget::OnQuitGameClicked()

{
    UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, true);
}

