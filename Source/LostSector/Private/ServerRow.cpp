// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRow.h"
#include "Components/Button.h"
#include "MainMenu.h"

void UServerRow::SetUp(UMainMenu* InParent, uint32 InIndex)
{
	Parent = InParent;
	SelfIndex = InIndex;
	RowButton->OnClicked.AddDynamic(this,&UServerRow::OnClicked);
}
void UServerRow::OnClicked()
{
	Parent->SetSelectedIndex(SelfIndex);
}