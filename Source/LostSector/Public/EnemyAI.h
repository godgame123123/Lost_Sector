// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "EnemyAI.generated.h"

/**
 * 
 */
UCLASS()
class LOSTSECTOR_API AEnemyAI : public AAIController
{
	GENERATED_BODY()

public:
    AEnemyAI();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

    // AI가 실행할 Behavior Tree 에셋을 블루프린트에서 설정하도록 함
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TObjectPtr<class UBehaviorTree> BehaviorTree;

    FTimerHandle LoseSightHandle;

    virtual void BeginPlay() override;
protected:
    // [핵심 수정] Pawn을 소유했을 때 호출되는 함수를 오버라이드합니다.
    virtual void OnPossess(APawn* InPawn) override;

private:
    // 인지(Perception) 이벤트 발생 시 호출될 함수 선언
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Blackboard 컴포넌트에 접근하기 위한 포인터
    TObjectPtr<UBlackboardComponent> BlackboardComp;


};
