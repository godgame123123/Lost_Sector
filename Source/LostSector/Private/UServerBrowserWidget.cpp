// Fill out your copyright notice in the Description page of Project Settings.

#include "UServerBrowserWidget.h"
#include "Components/PanelWidget.h"
#include "Components/Button.h"
#include "MyGameInstance.h"
#include "UServerBrowserItemWidget.h"

UServerBrowserWidget::UServerBrowserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 모든 위젯 클래스들은 런타임에 동적으로 로드하도록 변경 (패킹 에러 방지)
	// ConstructorHelpers::FClassFinder는 CDO 생성 시점에 에셋을 찾으려고 시도하므로
	// 패킹 과정에서 에셋이 없거나 경로가 잘못되면 실패합니다.
	// 따라서 ServerItemClass는 SetServerList()에서 동적으로 로드됩니다.
}

bool UServerBrowserWidget::Initialize()
{
	bool bSuccess = Super::Initialize();
	if (!bSuccess) return false;

	// 버튼 이벤트 바인딩
	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnBackClicked);
	}

	if (Btn_Refresh)
	{
		Btn_Refresh->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnRefreshClicked);
	}

	if (Btn_Join)
	{
		Btn_Join->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnJoinClicked);
	}

	// GameInstance 가져오기
	if (UWorld* World = GetWorld())
	{
		GameInstance = Cast<UMyGameInstance>(World->GetGameInstance());
	}

	return true;
}

void UServerBrowserWidget::SetOwningGameInstance(UMyGameInstance* InGameInstance)
{
	GameInstance = InGameInstance;
}

void UServerBrowserWidget::SetServerList(TArray<FServerData> InServerData)
{
	if (!ServerListPanel) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// ServerItemClass가 null이면 동적으로 로드 시도
	if (!ServerItemClass)
	{
		// 여러 경로 시도 (Blueprint 클래스는 _C 접미사 필요)
		static const TArray<FString> ServerItemPaths = {
			TEXT("/Game/Team_Folder/LHJ/UI/WB_ServerBrowser_Item.WB_ServerBrowser_Item_C")
		};

		for (const FString& Path : ServerItemPaths)
		{
			if (UClass* FoundClass = LoadClass<UUserWidget>(nullptr, *Path))
			{
				ServerItemClass = FoundClass;
				UE_LOG(LogTemp, Log, TEXT("[UServerBrowserWidget] SetServerList: WB_ServerBrowser_Item 클래스 동적 로드 성공 (경로: %s)"), *Path);
				break;
			}
		}

		if (!ServerItemClass)
		{
			UE_LOG(LogTemp, Error, TEXT("[UServerBrowserWidget] SetServerList: WB_ServerBrowser_Item Blueprint를 찾을 수 없습니다. 서버 목록을 표시할 수 없습니다."));
			return;
		}
	}

	// 기존 아이템 제거
	ServerListPanel->ClearChildren();

	// 새 서버 아이템 생성
	uint32 Index = 0;
	for (const FServerData& ServerData : InServerData)
	{
		if (!ServerItemClass) continue;

		UServerBrowserItemWidget* ServerItem = CreateWidget<UServerBrowserItemWidget>(World, ServerItemClass);
		if (!ServerItem) continue;

		// 서버 데이터 설정
		ServerItem->Setup(this, Index, ServerData);
		
		// 패널에 추가
		ServerListPanel->AddChild(ServerItem);
		
		Index++;
	}
}

void UServerBrowserWidget::SetSelectedIndex(uint32 InIndex)
{
	SelectedIndex = InIndex;

	// 모든 아이템의 선택 상태 업데이트
	if (!ServerListPanel) return;

	for (int32 i = 0; i < ServerListPanel->GetChildrenCount(); ++i)
	{
		if (UServerBrowserItemWidget* Item = Cast<UServerBrowserItemWidget>(ServerListPanel->GetChildAt(i)))
		{
			Item->SetSelected(SelectedIndex.IsSet() && SelectedIndex.GetValue() == (uint32)i);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[UServerBrowserWidget] SetSelectedIndex: 인덱스 %d 선택됨"), InIndex);
}

void UServerBrowserWidget::JoinSelectedServer()
{
	UE_LOG(LogTemp, Warning, TEXT("[UServerBrowserWidget] JoinSelectedServer: 호출됨"));
	UE_LOG(LogTemp, Warning, TEXT("[UServerBrowserWidget] JoinSelectedServer: SelectedIndex.IsSet() = %d"), SelectedIndex.IsSet() ? 1 : 0);
	UE_LOG(LogTemp, Warning, TEXT("[UServerBrowserWidget] JoinSelectedServer: GameInstance = %s"), GameInstance ? TEXT("Valid") : TEXT("Null"));
	
	if (SelectedIndex.IsSet() && GameInstance)
	{
		uint32 IndexToJoin = SelectedIndex.GetValue();
		UE_LOG(LogTemp, Warning, TEXT("[UServerBrowserWidget] JoinSelectedServer: 서버 조인 시작 - 인덱스: %d"), IndexToJoin);
		GameInstance->Join(IndexToJoin);
	}
	else
	{
		if (!SelectedIndex.IsSet())
		{
			UE_LOG(LogTemp, Error, TEXT("[UServerBrowserWidget] JoinSelectedServer: 서버가 선택되지 않았습니다!"));
		}
		if (!GameInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("[UServerBrowserWidget] JoinSelectedServer: GameInstance가 null입니다!"));
		}
	}
}

void UServerBrowserWidget::RefreshServerList() 
{
	if (GameInstance)
	{
		GameInstance->RefreshServerList();
	}
}

void UServerBrowserWidget::OnBackClicked()
{
	// 델리게이트 브로드캐스트
	OnBackClickedDelegate.Broadcast();
}

void UServerBrowserWidget::OnRefreshClicked()
{
	RefreshServerList();
}

void UServerBrowserWidget::OnJoinClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("[UServerBrowserWidget] OnJoinClicked: Join 버튼 클릭됨"));
	JoinSelectedServer();
}

