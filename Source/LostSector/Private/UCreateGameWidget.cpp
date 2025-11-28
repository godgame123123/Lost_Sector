// Fill out your copyright notice in the Description page of Project Settings.

#include "UCreateGameWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/CheckBox.h"
#include "Kismet/GameplayStatics.h"
#include "MyGameInstance.h"

UUCreateGameWidget::UUCreateGameWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, OwningInstance(nullptr)
{
}

bool UUCreateGameWidget::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;

	// 에디터에서 실행 중일 때는 델리게이트 바인딩 건너뛰기 (안전성)
	if (IsDesignTime())
	{
		return true;
	}

	// 위젯을 수동으로 찾기 (UPROPERTY(meta=(BindWidget)) 대신)
	CreateGameButton = Cast<UButton>(GetWidgetFromName(TEXT("CreateGameButton")));
	ServerNameInput = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("ServerNameInput")));
	PlayerAmountInput = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("PlayerAmountInput")));
	LanCheckBox = Cast<UCheckBox>(GetWidgetFromName(TEXT("LanCheckBox")));
	Btn_Back = Cast<UButton>(GetWidgetFromName(TEXT("Btn_Back")));

	// 위젯이 유효하고 nullptr이 아닌지 확인 후 바인딩
	if (CreateGameButton != nullptr && IsValid(CreateGameButton))
	{
		// 기존 바인딩 제거 (중복 방지)
		CreateGameButton->OnClicked.Clear();
		CreateGameButton->OnClicked.AddDynamic(this, &UUCreateGameWidget::HostGame);
	}

	if (Btn_Back != nullptr && IsValid(Btn_Back))
	{
		// 기존 바인딩 제거 (중복 방지)
		Btn_Back->OnClicked.Clear();
		Btn_Back->OnClicked.AddDynamic(this, &UUCreateGameWidget::OnBackClicked);
	}

	// GameInstance 가져오기 (에디터에서는 World가 없을 수 있음)
	if (!OwningInstance)
	{
		if (UWorld* World = GetWorld())
		{
			if (UMyGameInstance* GameInstance = Cast<UMyGameInstance>(World->GetGameInstance()))
			{
				SetOwningInstance(GameInstance);
			}
		}
	}

	return true;
}

void UUCreateGameWidget::SetOwningInstance(IMyInterface* InInstance)
{
	OwningInstance = InInstance;
}

void UUCreateGameWidget::HostGame()
{
	if (!OwningInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("OwningInstance is null! Cannot host game."));
		return;
	}

	FString ServerName = TEXT("My Server");
	FString PlayerCountString = TEXT("3"); // 기본값 3명

	// 서버 이름 입력
	if (ServerNameInput)
	{
		FString InputText = ServerNameInput->GetText().ToString();
		if (!InputText.IsEmpty())
		{
			ServerName = InputText;
		}
	}

	// 플레이어 수 입력
	if (PlayerAmountInput)
	{
		FString InputText = PlayerAmountInput->GetText().ToString();
		if (!InputText.IsEmpty())
		{
			PlayerCountString = InputText;
		}
	}

	int32 MaxPlayers = FCString::Atoi(*PlayerCountString);
	if (MaxPlayers < 2) MaxPlayers = 2; // 최소 2명
	if (MaxPlayers > 24) MaxPlayers = 24; // 최대 24명

	bool bIsLan = LanCheckBox ? LanCheckBox->IsChecked() : false;

	UE_LOG(LogTemp, Warning, TEXT("Host Game Request: Server Name = %s, Max Players = %d, Is LAN = %s"),
		*ServerName, MaxPlayers, bIsLan ? TEXT("True") : TEXT("False"));

	// 서버 호스팅 (MaxPlayers와 bIsLan 전달)
	OwningInstance->Host(ServerName, MaxPlayers, bIsLan);
}

void UUCreateGameWidget::OnBackClicked()
{
	// 백 버튼 델리게이트 브로드캐스트
	OnBackClickedDelegate.Broadcast();
}

		