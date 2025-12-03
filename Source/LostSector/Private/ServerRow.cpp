// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRow.h"
#include "Components/Button.h"
#include "Components/SlateWrapperTypes.h"
#include "MainMenu.h"

void UServerRow::SetUp(UMainMenu* InParent, int32 InIndex)
{
	Parent = InParent;
	SelfIndex = InIndex;
	
	UE_LOG(LogTemp, Log, TEXT("[ServerRow] SetUp: 인덱스 %d 초기화 시작"), InIndex);
	
	if (RowButton)
	{
		// 버튼이 클릭 가능하도록 설정
		RowButton->SetIsEnabled(true);
		RowButton->SetVisibility(ESlateVisibility::Visible);
		
		// 기존 바인딩 제거 후 새로 바인딩
		RowButton->OnClicked.Clear();
		RowButton->OnClicked.AddDynamic(this, &UServerRow::OnClicked);
		
		UE_LOG(LogTemp, Log, TEXT("[ServerRow] SetUp: RowButton 바인딩 완료 (인덱스 %d, Enabled: %d, Visible: %d)"), 
			InIndex, RowButton->GetIsEnabled() ? 1 : 0, 
			RowButton->GetVisibility() == ESlateVisibility::Visible ? 1 : 0);
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

void UServerRow::UpdateSelectionState(bool bIsSelected)
{
	bSelected = bIsSelected;
	
	// 버튼의 시각적 피드백 업데이트
	if (RowButton)
	{
		// 선택된 경우 버튼을 강조 표시 (색상 변경 등)
		// Blueprint에서 구현할 수도 있지만, 기본적인 시각적 피드백 제공
		if (bIsSelected)
		{
			// 선택된 상태: 버튼을 활성화 상태로 유지
			RowButton->SetIsEnabled(true);
			UE_LOG(LogTemp, Log, TEXT("[ServerRow] UpdateSelectionState: 인덱스 %d 선택됨"), SelfIndex);
		}
		else
		{
			// 선택되지 않은 상태
			RowButton->SetIsEnabled(true);
		}
	}
}