// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UMiniMapWidget.generated.h"

class UImage;
class UCanvasPanel;
class UTextBlock;

/**
 * 미니맵 위젯 클래스
 * Scene Capture 2D로 렌더링된 맵 이미지를 표시하고,
 * 플레이어 위치와 다른 오브젝트들을 오버레이로 표시합니다.
 */
UCLASS()
class LOSTSECTOR_API UMiniMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 미니맵 이미지 설정 (Render Target)
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void SetMiniMapTexture(UTexture2D* Texture);

	// 미니맵 Material 설정
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void SetMiniMapMaterial(UMaterialInterface* Material);

	// 플레이어 위치 업데이트
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void UpdatePlayerPosition(const FVector2D& WorldPosition, float Rotation);

	// 다른 플레이어 위치 업데이트
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void UpdateOtherPlayerPosition(int32 PlayerIndex, const FVector2D& WorldPosition, float Rotation);

	// 몬스터 위치 업데이트
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void UpdateEnemyPosition(int32 EnemyIndex, const FVector2D& WorldPosition);

	// 미니맵 범위 설정 (월드 좌표)
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void SetMapBounds(const FVector2D& MinBounds, const FVector2D& MaxBounds);

protected:
	// 미니맵 배경 이미지 (Render Target)
	UPROPERTY(meta = (BindWidget))
	UImage* MiniMapImage;

	// 플레이어 아이콘을 표시할 캔버스
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* IconsCanvas;

	// 플레이어 아이콘 이미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MiniMap")
	UTexture2D* PlayerIconTexture;

	// 다른 플레이어 아이콘 이미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MiniMap")
	UTexture2D* OtherPlayerIconTexture;

	// 몬스터 아이콘 이미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MiniMap")
	UTexture2D* EnemyIconTexture;

	// 아이콘 크기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MiniMap")
	FVector2D IconSize = FVector2D(20.0f, 20.0f);

	// 미니맵 범위 (월드 좌표)
	UPROPERTY(BlueprintReadOnly, Category = "MiniMap")
	FVector2D MapMinBounds = FVector2D(-5000.0f, -5000.0f);

	UPROPERTY(BlueprintReadOnly, Category = "MiniMap")
	FVector2D MapMaxBounds = FVector2D(5000.0f, 5000.0f);

	// 미니맵 크기 (픽셀)
	UPROPERTY(BlueprintReadOnly, Category = "MiniMap")
	FVector2D MiniMapSize = FVector2D(200.0f, 200.0f);

private:
	// 플레이어 아이콘 이미지 위젯
	UPROPERTY()
	UImage* PlayerIconWidget;

	// 다른 플레이어 아이콘들
	UPROPERTY()
	TMap<int32, UImage*> OtherPlayerIcons;

	// 몬스터 아이콘들
	UPROPERTY()
	TMap<int32, UImage*> EnemyIcons;

	// 월드 좌표를 미니맵 UI 좌표로 변환
	FVector2D WorldToMiniMapPosition(const FVector2D& WorldPosition) const;

	// 아이콘 생성
	UImage* CreateIcon(UTexture2D* IconTexture, const FVector2D& Position);
};

