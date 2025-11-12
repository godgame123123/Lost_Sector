// Fill out your copyright notice in the Description page of Project Settings.
#include "EnemyAI.h"
#include "Perception/AISense_Sight.h" // 시야 감지 사용
#include "Perception/AISense_Hearing.h" // 청각 감지 사용
#include "TimerManager.h"


AEnemyAI::AEnemyAI()
{
    // 1. AIPerceptionComponent 생성 및 설정
    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
    SetPerceptionComponent(*AIPerceptionComp);
}

void AEnemyAI::BeginPlay()
{
    Super::BeginPlay();


    if (AIPerceptionComp)
    {
       AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAI::OnTargetPerceptionUpdated);
    }

}

void AEnemyAI::OnPossess(APawn * InPawn)
{
    Super::OnPossess(InPawn);

    // BlackboardComp를 초기화하기 전에 RunBehaviorTree를 호출합니다.
    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
        BlackboardComp = GetBlackboardComponent();

}

    void AEnemyAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor && Actor->ActorHasTag(TEXT("Player")) && BlackboardComp)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // 최종 수정 로직: GetDefaultObject()를 통해 GetSenseID()를 호출

            // 1. 시야 감지 확인
            if (Stimulus.Type == GetDefault<UAISense_Sight>()->GetSenseID())
            {
                GetWorld()->GetTimerManager().ClearTimer(LoseSightHandle);
                // 타겟 액터와 위치 저장 (전투 상태 유도)
                BlackboardComp->SetValueAsObject(FName("TargetActor"), Actor);
                BlackboardComp->ClearValue(FName("LastKnownLocation"));
            }
            // 2. 청각 감지 확인
            else if (Stimulus.Type == GetDefault<UAISense_Hearing>()->GetSenseID())
            {
                // 소리 발생 위치만 저장 (경계/수색 상태 유도)
                BlackboardComp->SetValueAsVector(FName("LastKnownLocation"), Stimulus.StimulusLocation);
            }
        }
        else // 감지를 놓쳤을 때
        {
            GetWorld()->GetTimerManager().SetTimer(
                LoseSightHandle,
                [this, Actor]()
                {
                    // 일정 시간 후 TargetActor 제거, 마지막 위치 저장
                    if (BlackboardComp)
                    {
                        BlackboardComp->ClearValue(FName("TargetActor"));
                        BlackboardComp->SetValueAsVector(
                            FName("LastKnownLocation"),
                            Actor->GetActorLocation()
                        );
                    }
                },
                6.0f,   // 타임리밋 지속 시간 (원하는 값으로 변경 가능)
                false
            );
        }
    }
}
