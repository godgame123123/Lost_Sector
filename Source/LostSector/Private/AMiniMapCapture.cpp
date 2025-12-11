// Fill out your copyright notice in the Description page of Project Settings.

#include "AMiniMapCapture.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Camera/CameraComponent.h"

AMiniMapCapture::AMiniMapCapture()
{
	PrimaryActorTick.bCanEverTick = true;

	// Scene Capture 컴포넌트 생성
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
	RootComponent = SceneCaptureComponent;

	// 기본 설정
	CameraHeight = 5000.0f;
	CameraFOV = 90.0f;
}

void AMiniMapCapture::BeginPlay()
{
	Super::BeginPlay();

	// Render Target이 없으면 생성
	if (!RenderTarget)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this);
		if (RenderTarget)
		{
			RenderTarget->InitAutoFormat(512, 512); // 512x512 해상도
			RenderTarget->UpdateResourceImmediate(true);
		}
	}

	// Scene Capture 설정
	if (SceneCaptureComponent)
	{
		SceneCaptureComponent->TextureTarget = RenderTarget;
		SceneCaptureComponent->CaptureSource = SCS_FinalColorLDR;
		SceneCaptureComponent->bCaptureEveryFrame = true;
		SceneCaptureComponent->bCaptureOnMovement = true;
		
		// 위에서 내려다보는 각도로 설정
		FRotator CameraRotation = FRotator(-90.0f, 0.0f, 0.0f);
		SceneCaptureComponent->SetWorldRotation(CameraRotation);
		SceneCaptureComponent->FOVAngle = CameraFOV;
	}

	// 초기 카메라 위치 설정
	UpdateCameraPosition();
}

void AMiniMapCapture::SetTarget(AActor* NewTarget)
{
	TargetActor = NewTarget;
	UpdateCameraPosition();
}

void AMiniMapCapture::UpdateCameraPosition()
{
	if (!TargetActor || !SceneCaptureComponent)
	{
		return;
	}

	FVector TargetLocation = TargetActor->GetActorLocation();
	FVector CameraLocation = FVector(TargetLocation.X, TargetLocation.Y, TargetLocation.Z + CameraHeight);
	
	SceneCaptureComponent->SetWorldLocation(CameraLocation);
}

void AMiniMapCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 타겟을 따라 카메라 위치 업데이트
	if (TargetActor)
	{
		UpdateCameraPosition();
	}
}

