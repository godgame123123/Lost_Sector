// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "ServerRow.h"
#include "MyGameInstance.h"
#include "Framework/Application/SlateApplication.h"

UMainMenu::UMainMenu()
{
	// WB_ServerRow 위젯 클래스 찾기
	ConstructorHelpers::FClassFinder<UUserWidget> ServerRowClass_Asset(TEXT("/Game/Team_Folder/GimanLee/WB_ServerRow"));
	if (ServerRowClass_Asset.Succeeded())
	{
		ServerRowClass = ServerRowClass_Asset.Class;
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Constructor: WB_ServerRow 클래스 로드 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] Constructor: WB_ServerRow 클래스를 찾을 수 없습니다! 경로: /Game/Team_Folder/GimanLee/WB_ServerRow"));
		// 대체 경로 시도
		ConstructorHelpers::FClassFinder<UUserWidget> ServerRowClass_Asset2(TEXT("/Game/UI/WB_ServerRow"));
		if (ServerRowClass_Asset2.Succeeded())
		{
			ServerRowClass = ServerRowClass_Asset2.Class;
			UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Constructor: 대체 경로(/Game/UI/WB_ServerRow)에서 WB_ServerRow 클래스 로드 성공"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[MainMenu] Constructor: 모든 경로에서 WB_ServerRow 클래스를 찾을 수 없습니다!"));
		}
	}

		//Init() ;
}

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();
	
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] NativeConstruct: 위젯 생성 완료, 버튼 바인딩 재시도"));
	
	// NativeConstruct에서도 버튼 바인딩 재시도 (Initialize가 너무 일찍 호출될 수 있음)
	if (HostButton != nullptr)
	{
		// 버튼 상태 확인
		bool bButtonEnabled = HostButton->GetIsEnabled();
		ESlateVisibility ButtonVisibility = HostButton->GetVisibility();
		bool bIsVisible = HostButton->IsVisible();
		
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] NativeConstruct: HostButton 상태 - Enabled: %d, Visibility: %d, IsVisible: %d"), 
			bButtonEnabled ? 1 : 0, (int32)ButtonVisibility, bIsVisible ? 1 : 0);
		
		// 바인딩 전 상태 확인
		int32 BeforeCount = HostButton->OnPressed.GetAllObjects().Num();
		HostButton->OnPressed.AddDynamic(this, &UMainMenu::OpenHostMenu);
		int32 AfterCount = HostButton->OnPressed.GetAllObjects().Num();
		
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] NativeConstruct: HostButton 바인딩 완료 (바인딩 전: %d, 바인딩 후: %d)"), BeforeCount, AfterCount);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] NativeConstruct: HostButton이 null입니다! Blueprint에서 버튼 이름이 'HostButton'인지 확인하세요."));
	}
	
	if (JoinButton != nullptr)
	{
		// 버튼 상태 확인
		bool bButtonEnabled = JoinButton->GetIsEnabled();
		ESlateVisibility ButtonVisibility = JoinButton->GetVisibility();
		bool bIsVisible = JoinButton->IsVisible();
		
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] NativeConstruct: JoinButton 상태 - Enabled: %d, Visibility: %d, IsVisible: %d"), 
			bButtonEnabled ? 1 : 0, (int32)ButtonVisibility, bIsVisible ? 1 : 0);
		
		// 바인딩 전 상태 확인
		int32 BeforeCount = JoinButton->OnPressed.GetAllObjects().Num();
		JoinButton->OnPressed.AddDynamic(this, &UMainMenu::OpenJoinMenu);
		int32 AfterCount = JoinButton->OnPressed.GetAllObjects().Num();
		
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] NativeConstruct: JoinButton 바인딩 완료 (바인딩 전: %d, 바인딩 후: %d)"), BeforeCount, AfterCount);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] NativeConstruct: JoinButton이 null입니다! Blueprint에서 버튼 이름이 'JoinButton'인지 확인하세요."));
	}
}

bool UMainMenu::Initialize()
{
	bool bSuccess = Super::Initialize();
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] Initialize: Super::Initialize() 실패"));
	return false;
	}

	// 디자인 타임에서는 바인딩하지 않음
	if (IsDesignTime())
	{
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: 디자인 타임 모드 - 바인딩 건너뜀"));
		return true;
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: 버튼 바인딩 시작"));

	if (HostButton != nullptr)
	{
		// 버튼 상태 확인
		bool bButtonEnabled = HostButton->GetIsEnabled();
		ESlateVisibility ButtonVisibility = HostButton->GetVisibility();
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: HostButton 상태 - Enabled: %d, Visibility: %d"), 
			bButtonEnabled ? 1 : 0, (int32)ButtonVisibility);
		
		// 버튼이 활성화되어 있지 않으면 활성화
		if (!bButtonEnabled)
		{
			HostButton->SetIsEnabled(true);
			UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: HostButton이 비활성화되어 있었습니다. 활성화했습니다."));
		}
		
		// 버튼이 보이지 않으면 보이게 설정
		if (ButtonVisibility != ESlateVisibility::Visible)
		{
			HostButton->SetVisibility(ESlateVisibility::Visible);
			UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: HostButton이 보이지 않았습니다. 보이게 설정했습니다."));
		}
		
		// 기존 바인딩 확인
		int32 BeforeCount = HostButton->OnPressed.GetAllObjects().Num();
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: HostButton 기존 바인딩 수: %d"), BeforeCount);
		
		HostButton->OnPressed.AddDynamic(this, &UMainMenu::OpenHostMenu);
		
		// 바인딩 확인
		int32 AfterCount = HostButton->OnPressed.GetAllObjects().Num();
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: HostButton 바인딩 완료 (바인딩 전: %d, 바인딩 후: %d)"), BeforeCount, AfterCount);
		
		// 버튼 클릭 가능 여부 확인
		bool bCanClick = HostButton->GetIsEnabled() && 
		                 HostButton->GetVisibility() == ESlateVisibility::Visible &&
		                 HostButton->IsVisible();
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: HostButton 클릭 가능 여부: %d (Enabled: %d, Visible: %d, IsVisible: %d)"), 
			bCanClick ? 1 : 0,
			HostButton->GetIsEnabled() ? 1 : 0,
			HostButton->GetVisibility() == ESlateVisibility::Visible ? 1 : 0,
			HostButton->IsVisible() ? 1 : 0);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: HostButton이 null입니다!"));
	}
	
	if (JoinButton != nullptr)
	{
		// 버튼 상태 확인
		bool bButtonEnabled = JoinButton->GetIsEnabled();
		ESlateVisibility ButtonVisibility = JoinButton->GetVisibility();
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: JoinButton 상태 - Enabled: %d, Visibility: %d"), 
			bButtonEnabled ? 1 : 0, (int32)ButtonVisibility);
		
		// 버튼이 활성화되어 있지 않으면 활성화
		if (!bButtonEnabled)
		{
			JoinButton->SetIsEnabled(true);
			UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: JoinButton이 비활성화되어 있었습니다. 활성화했습니다."));
		}
		
		// 버튼이 보이지 않으면 보이게 설정
		if (ButtonVisibility != ESlateVisibility::Visible)
		{
			JoinButton->SetVisibility(ESlateVisibility::Visible);
			UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: JoinButton이 보이지 않았습니다. 보이게 설정했습니다."));
		}
		
		JoinButton->OnPressed.AddDynamic(this, &UMainMenu::OpenJoinMenu);
		
		// 바인딩 확인
		int32 BoundCount = JoinButton->OnPressed.GetAllObjects().Num();
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: JoinButton 바인딩 완료 (바인딩된 핸들러 수: %d)"), BoundCount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: JoinButton이 null입니다!"));
	}

	if (CancelHostButton != nullptr)
	{
		CancelHostButton->OnPressed.AddDynamic(this, &UMainMenu::OpenMainMenu);
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: CancelHostButton 바인딩 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: CancelHostButton이 null입니다!"));
	}
	
	if (CancelJoinButton != nullptr)
	{
		CancelJoinButton->OnPressed.AddDynamic(this, &UMainMenu::OpenMainMenu);
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: CancelJoinButton 바인딩 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: CancelJoinButton이 null입니다!"));
	}

	if (ConfirmHostButton != nullptr)
	{
		ConfirmHostButton->OnPressed.AddDynamic(this, &UMainMenu::HostServer);
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: ConfirmHostButton 바인딩 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: ConfirmHostButton이 null입니다!"));
	}

	if (QuitButton != nullptr)
	{
		QuitButton->OnPressed.AddDynamic(this, &UMainMenu::QuitGame);
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: QuitButton 바인딩 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: QuitButton이 null입니다!"));
	}

	if (ConfirmJoinButton != nullptr)
	{
		ConfirmJoinButton->OnPressed.AddDynamic(this, &UMainMenu::JoinServer);
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: ConfirmJoinButton 바인딩 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: ConfirmJoinButton이 null입니다!"));
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Initialize: 모든 버튼 바인딩 완료"));
	
	// Input Mode 확인
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			// GetInputMode()는 존재하지 않으므로 제거하고 마우스 커서 상태만 확인
			UE_LOG(LogTemp, Warning, TEXT("[MainMenu] Initialize: InputMode 확인 - MouseCursor: %d"), PC->bShowMouseCursor ? 1 : 0);
		}
	}
	
	return true;
}

FReply UMainMenu::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("[MainMenu] NativeOnMouseButtonDown: 마우스 클릭 감지됨! 버튼: %s"), 
		InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton ? TEXT("Left") : TEXT("Other"));
	
	// 부모 클래스의 기본 동작 수행
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
void UMainMenu::QuitGame()
{
	UE_LOG(LogTemp, Warning, TEXT("[MainMenu] QuitGame 버튼 클릭됨"));
	
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] QuitGame: World가 null입니다!"));
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] QuitGame: PlayerController가 null입니다!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] QuitGame: 게임 종료 명령 실행"));
	PC->ConsoleCommand("quit");
}

void UMainMenu::JoinServer()
{
	UE_LOG(LogTemp, Warning, TEXT("[MainMenu] JoinServer 버튼 클릭됨"));

	if (SelectedIndex.IsSet() && OwningInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] JoinServer: 선택된 서버 인덱스 = %d"), SelectedIndex.GetValue());
		OwningInstance->Join(SelectedIndex.GetValue());
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] JoinServer: 서버 조인 요청 완료"));
	}
	else
	{
		if (!SelectedIndex.IsSet())
		{
			UE_LOG(LogTemp, Warning, TEXT("[MainMenu] JoinServer: 서버가 선택되지 않았습니다!"));
		}
		if (!OwningInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("[MainMenu] JoinServer: OwningInstance가 null입니다!"));
		}
	}
}

void UMainMenu::SetSelectedIndex(uint32 InIndex)
{
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] SetSelectedIndex: 인덱스 %d 선택됨"), InIndex);
	
	if (!Serverlist)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] SetSelectedIndex: Serverlist가 null입니다!"));
		return;
	}
	
	SelectedIndex = InIndex;
	int32 SelectedCount = 0;
	for (int32 i = 0; i < Serverlist->GetChildrenCount(); ++i)
	{
		auto serverRow = Cast<UServerRow>(Serverlist->GetChildAt(i));
		if (serverRow)
		{
			bool bWasSelected = serverRow->bSelected;
			serverRow->bSelected = 
				(SelectedIndex.IsSet() && 
					SelectedIndex.GetValue() == i);
			
			if (serverRow->bSelected)
			{
				SelectedCount++;
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] SetSelectedIndex: 선택 완료 (선택된 항목 수: %d)"), SelectedCount);
}

void UMainMenu::SetServerList(
	TArray<FServerData> InServerData)
{
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] SetServerList: 서버 목록 설정 시작 (서버 수: %d)"), InServerData.Num());
	
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] SetServerList: World가 null입니다!"));
		return;
	}

	if (!Serverlist)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] SetServerList: Serverlist가 null입니다!"));
		return;
	}

	if (!ServerRowClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] SetServerList: ServerRowClass가 null입니다! WB_ServerRow 위젯을 찾을 수 없습니다."));
		return;
	}

	Serverlist->ClearChildren();
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] SetServerList: 기존 서버 목록 초기화 완료"));

	uint32 i = 0;
	uint32 SuccessCount = 0;
	for (const FServerData& ServerData : InServerData)
	{
		UServerRow* ServerRow = CreateWidget<UServerRow>(World, ServerRowClass);
		if (!ServerRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MainMenu] SetServerList: 서버 행 생성 실패 (인덱스 %d, 서버명: %s)"), i, *ServerData.Name);
			continue; // 이 서버 항목을 건너뛰고 다음으로
		}

		ServerRow->ServerName->SetText(FText::FromString(ServerData.Name));
		ServerRow->HostUser->SetText(FText::FromString(ServerData.HostUserName));

		FString FractionText = FString::Printf(TEXT("%d/%d"), ServerData.CurrentPlayers, ServerData.MaxPlayers);
		ServerRow->ConnectionFraction->SetText(FText::FromString(FractionText));
		ServerRow->SetUp(this, i++);
		Serverlist->AddChild(ServerRow);
		SuccessCount++;
		
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] SetServerList: 서버 추가 완료 - 이름: %s, 호스트: %s, 플레이어: %d/%d"), 
			*ServerData.Name, *ServerData.HostUserName, ServerData.CurrentPlayers, ServerData.MaxPlayers);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] SetServerList: 서버 목록 설정 완료 (성공: %d/%d)"), SuccessCount, InServerData.Num());
}

void UMainMenu::OpenMainMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("[MainMenu] OpenMainMenu 버튼 클릭됨"));
	
	if (!MenuSwitcher)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] OpenMainMenu: MenuSwitcher가 null입니다!"));
		return;
	}
	if (!MainMenu)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] OpenMainMenu: MainMenu가 null입니다!"));
		return;
	}

	MenuSwitcher->SetActiveWidget(MainMenu);
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] OpenMainMenu: 메인 메뉴로 전환 완료"));
}
void UMainMenu::OpenHostMenu()
{
	// 중단점이 작동하는지 확인하기 위한 강제 로그
	UE_LOG(LogTemp, Error, TEXT("[MainMenu] ========== OpenHostMenu 함수 호출됨! =========="));
	
	// 중단점 테스트용 변수
	int32 TestValue = 42;
	TestValue++; // 여기에 중단점 설정
	
	UE_LOG(LogTemp, Warning, TEXT("[MainMenu] ========== OpenHostMenu 버튼 클릭됨 =========="));
	
	// 버튼 상태 재확인
	if (HostButton)
	{
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] OpenHostMenu: HostButton 상태 - Enabled: %d, Visible: %d"), 
			HostButton->GetIsEnabled() ? 1 : 0, 
			HostButton->GetVisibility() == ESlateVisibility::Visible ? 1 : 0);
	}
	
	if (!MenuSwitcher)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] OpenHostMenu: MenuSwitcher가 null입니다!"));
		return;
	}
	if (!HostMenu)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] OpenHostMenu: HostMenu가 null입니다!"));
		return;
	}
	
	MenuSwitcher->SetActiveWidget(HostMenu);
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] OpenHostMenu: 호스트 메뉴로 전환 완료"));
}
void UMainMenu::OpenJoinMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("[MainMenu] ========== OpenJoinMenu 버튼 클릭됨 =========="));
	
	// 버튼 상태 재확인
	if (JoinButton)
	{
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] OpenJoinMenu: JoinButton 상태 - Enabled: %d, Visible: %d"), 
			JoinButton->GetIsEnabled() ? 1 : 0, 
			JoinButton->GetVisibility() == ESlateVisibility::Visible ? 1 : 0);
	}
	
	if (!MenuSwitcher)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] OpenJoinMenu: MenuSwitcher가 null입니다!"));
		return;
	}
	if (!JoinMenu)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] OpenJoinMenu: JoinMenu가 null입니다!"));
		return;
	}

	MenuSwitcher->SetActiveWidget(JoinMenu);
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] OpenJoinMenu: 조인 메뉴로 전환 완료"));

	if (OwningInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("[MainMenu] OpenJoinMenu: 서버 목록 새로고침 시작"));
		OwningInstance->RefreshServerList();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] OpenJoinMenu: OwningInstance가 null입니다!"));
	}
}

void UMainMenu::HostServer()
{
	UE_LOG(LogTemp, Warning, TEXT("[MainMenu] HostServer 버튼 클릭됨"));
	
	if (!ServerHostName)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] HostServer: ServerHostName이 null입니다!"));
		return;
	}
	if (!OwningInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenu] HostServer: OwningInstance가 null입니다!"));
		return;
	}

	FString ServerName = ServerHostName->GetText().ToString();
	if (ServerName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] HostServer: 서버 이름이 비어있습니다!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] HostServer: 서버 호스팅 시작 - 서버 이름: %s"), *ServerName);
	OwningInstance->Host(ServerName);
}

