// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FireWeapon.generated.h"

/**
 * 
 */
UCLASS()
class LOSTSECTOR_API UBTT_FireWeapon : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTT_FireWeapon();

	// 공격 지속 시간 (에디터에서 설정)
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDuration = 2.0f;

	// 타겟 위치 블랙보드 키 (Player Vector Location)
	// FBlackboardKeySelector를 사용하면 에디터에서 키 이름을 선택할 수 있습니다.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;

protected:
	// Task 시작 시 호출됨
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Task가 InProgress 상태일 때 매 틱(Tick)마다 호출됨
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// 공격 경과 시간 저장
	float AttackTimeElapsed;
};
