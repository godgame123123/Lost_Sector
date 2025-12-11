// Fill out your copyright notice in the Description page of Project Settings.

#include "UMiniMapWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

void UMiniMapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 플레이어 아이콘 생성
	if (PlayerIconTexture && IconsCanvas)
	{
		PlayerIconWidget = CreateIcon(PlayerIconTexture, FVector2D::ZeroVector);
		if (PlayerIconWidget)
		{
			IconsCanvas->AddChild(PlayerIconWidget);
		}
	}
}

void UMiniMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UMiniMapWidget::SetMiniMapTexture(UTexture2D* Texture)
{
	if (MiniMapImage && Texture)
	{
		MiniMapImage->SetBrushFromTexture(Texture);
	}
}

void UMiniMapWidget::SetMiniMapMaterial(UMaterialInterface* Material)
{
	if (MiniMapImage && Material)
	{
		MiniMapImage->SetBrushFromMaterial(Material);
	}
}

void UMiniMapWidget::UpdatePlayerPosition(const FVector2D& WorldPosition, float Rotation)
{
	FVector2D MiniMapPos = WorldToMiniMapPosition(WorldPosition);
	
	if (PlayerIconWidget && IconsCanvas)
	{
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(PlayerIconWidget->Slot);
		if (CanvasSlot)
		{
			CanvasSlot->SetPosition(MiniMapPos - IconSize * 0.5f);
			CanvasSlot->SetSize(IconSize);
			
			// 회전 적용
			PlayerIconWidget->SetRenderTransformAngle(Rotation);
		}
	}
}

void UMiniMapWidget::UpdateOtherPlayerPosition(int32 PlayerIndex, const FVector2D& WorldPosition, float Rotation)
{
	FVector2D MiniMapPos = WorldToMiniMapPosition(WorldPosition);
	
	if (!OtherPlayerIcons.Contains(PlayerIndex))
	{
		if (OtherPlayerIconTexture && IconsCanvas)
		{
			UImage* Icon = CreateIcon(OtherPlayerIconTexture, MiniMapPos);
			if (Icon)
			{
				IconsCanvas->AddChild(Icon);
				OtherPlayerIcons.Add(PlayerIndex, Icon);
			}
		}
	}
	
	if (UImage* Icon = OtherPlayerIcons.FindRef(PlayerIndex))
	{
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Icon->Slot);
		if (CanvasSlot)
		{
			CanvasSlot->SetPosition(MiniMapPos - IconSize * 0.5f);
			CanvasSlot->SetSize(IconSize);
			Icon->SetRenderTransformAngle(Rotation);
		}
	}
}

void UMiniMapWidget::UpdateEnemyPosition(int32 EnemyIndex, const FVector2D& WorldPosition)
{
	FVector2D MiniMapPos = WorldToMiniMapPosition(WorldPosition);
	
	if (!EnemyIcons.Contains(EnemyIndex))
	{
		if (EnemyIconTexture && IconsCanvas)
		{
			UImage* Icon = CreateIcon(EnemyIconTexture, MiniMapPos);
			if (Icon)
			{
				IconsCanvas->AddChild(Icon);
				EnemyIcons.Add(EnemyIndex, Icon);
			}
		}
	}
	
	if (UImage* Icon = EnemyIcons.FindRef(EnemyIndex))
	{
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Icon->Slot);
		if (CanvasSlot)
		{
			CanvasSlot->SetPosition(MiniMapPos - IconSize * 0.5f);
			CanvasSlot->SetSize(IconSize);
		}
	}
}

void UMiniMapWidget::SetMapBounds(const FVector2D& MinBounds, const FVector2D& MaxBounds)
{
	MapMinBounds = MinBounds;
	MapMaxBounds = MaxBounds;
}

FVector2D UMiniMapWidget::WorldToMiniMapPosition(const FVector2D& WorldPosition) const
{
	// 월드 좌표를 0-1 범위로 정규화
	float NormalizedX = (WorldPosition.X - MapMinBounds.X) / (MapMaxBounds.X - MapMinBounds.X);
	float NormalizedY = (WorldPosition.Y - MapMinBounds.Y) / (MapMaxBounds.Y - MapMinBounds.Y);
	
	// 미니맵 크기에 맞게 변환
	FVector2D MiniMapPos;
	MiniMapPos.X = NormalizedX * MiniMapSize.X;
	MiniMapPos.Y = (1.0f - NormalizedY) * MiniMapSize.Y; // Y축 반전 (UI는 위에서 아래로)
	
	return MiniMapPos;
}

UImage* UMiniMapWidget::CreateIcon(UTexture2D* IconTexture, const FVector2D& Position)
{
	if (!IconTexture || !IconsCanvas)
	{
		return nullptr;
	}
	
	UImage* Icon = NewObject<UImage>(this);
	if (Icon)
	{
		Icon->SetBrushFromTexture(IconTexture);
		Icon->SetVisibility(ESlateVisibility::Visible);
		
		// Canvas에 추가하고 Slot 설정
		IconsCanvas->AddChild(Icon);
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Icon->Slot))
		{
			CanvasSlot->SetPosition(Position - IconSize * 0.5f);
			CanvasSlot->SetSize(IconSize);
		}
	}
	
	return Icon;
}

