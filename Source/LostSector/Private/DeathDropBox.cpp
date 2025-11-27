// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathDropBox.h"
#include "InventoryComponent.h"

ADeathDropBox::ADeathDropBox()
{
}

void ADeathDropBox::InitializeLoot(const TArray<FItemStack>& InventoryToStore)
{
	Super::InitializeLoot(InventoryToStore);
}
