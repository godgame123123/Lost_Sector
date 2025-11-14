// Fill out your copyright notice in the Description page of Project Settings.


#include "UIManagerSubsystem.h"


void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("UIManager Subsystem Initialized!"));
}

void UUIManagerSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("UIManager Subsystem Deinitialized."));

	Super::Deinitialize();
}
