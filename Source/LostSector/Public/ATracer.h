// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "ATracer.generated.h"

UCLASS()
class LOSTSECTOR_API AATracer : public AActor
{
	GENERATED_BODY()
	
public:
	AATracer();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ----------------------------------------------------
	// [추가된 부분]
	// ----------------------------------------------------

private:
	// 나이아가라 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> NiagaraComp;

	// 이동 관련 변수
	FVector StartLocation;
	FVector TargetLocation;
	float TotalFlightTime = 0.0f; // 이동에 걸리는 총 시간 (AWeapon에서 설정)
	float ElapsedTime = 0.0f;     // 경과 시간

public:
	/** 무기에서 호출하여 트레이서의 이동을 시작하는 함수 */
	UFUNCTION(BlueprintCallable, Category = "Tracer")
	void StartMoving(const FVector Target, float Speed);

	/** AWeapon::PerformLineTrace에서 사용할 수 있도록 컴포넌트 getter 추가 */
	FORCEINLINE class UNiagaraComponent* GetNiagaraComp() const { return NiagaraComp; }
};
