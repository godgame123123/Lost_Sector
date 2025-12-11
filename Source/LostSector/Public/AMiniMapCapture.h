// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "AMiniMapCapture.generated.h"

class USceneCaptureComponent2D;

/**
 * 미니맵을 위한 Scene Capture 액터
 * 위에서 내려다보는 카메라로 씬을 렌더링하여 Render Target에 저장합니다.
 */
UCLASS()
class LOSTSECTOR_API AMiniMapCapture : public AActor
{
	GENERATED_BODY()
	
public:	
	AMiniMapCapture();

protected:
	virtual void BeginPlay() override;

	// Scene Capture 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MiniMap")
	USceneCaptureComponent2D* SceneCaptureComponent;

	// Render Target 텍스처
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap")
	UTextureRenderTarget2D* RenderTarget;

	// 카메라 높이 (Z축)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap")
	float CameraHeight = 5000.0f;

	// 카메라 FOV (Field of View)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap")
	float CameraFOV = 90.0f;

	// 추적할 타겟 (플레이어 캐릭터)
	UPROPERTY(BlueprintReadWrite, Category = "MiniMap")
	AActor* TargetActor;

public:
	// 타겟 설정
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void SetTarget(AActor* NewTarget);

	// Render Target 가져오기
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	// 카메라 위치 업데이트 (타겟을 따라감)
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void UpdateCameraPosition();

	virtual void Tick(float DeltaTime) override;
};

