// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRow.h"
#include "Components/Button.h"
#include "MainMenu.h"

void UServerRow::SetUp(UMainMenu* InParent, int32 InIndex)
{
	Parent = InParent;
	SelfIndex = InIndex;
	
	UE_LOG(LogTemp, Log, TEXT("[ServerRow] SetUp: 인덱스 %d 초기화 시작"), InIndex);
	
	if (RowButton)
	{
		RowButton->OnClicked.Clear();
		RowButton->OnClicked.AddDynamic(this, &UServerRow::OnClicked);
		UE_LOG(LogTemp, Log, TEXT("[ServerRow] SetUp: RowButton 바인딩 완료 (인덱스 %d)"), InIndex);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ServerRow] SetUp: RowButton이 null입니다! (인덱스 %d)"), InIndex);
	}
}
void UServerRow::OnClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("[ServerRow] OnClicked: 서버 행 클릭됨 (인덱스 %d)"), SelfIndex);
	
	// 블루프린트 이벤트 호출
	OnServerRowClicked();
	
	if (Parent)
	{
		UE_LOG(LogTemp, Log, TEXT("[ServerRow] OnClicked: Parent->SetSelectedIndex 호출 (인덱스 %d)"), SelfIndex);
		Parent->SetSelectedIndex(SelfIndex);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ServerRow] OnClicked: Parent가 null입니다! (인덱스 %d)"), SelfIndex);
	}
}