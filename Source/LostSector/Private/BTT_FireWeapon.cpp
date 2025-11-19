// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FireWeapon.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "LostSector/LostSectorCharacter.h" 
#include "Kismet/KismetMathLibrary.h"

UBTT_FireWeapon::UBTT_FireWeapon()
{
	NodeName = "BTT FireWeapon";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_FireWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ALostSectorCharacter* AICharacter = AIController ? Cast<ALostSectorCharacter>(AIController->GetPawn()) : nullptr;

	if (!AICharacter)
	{
		return EBTNodeResult::Failed;
	}

	// 1. 연사 시작 (ALostSectorCharacter에서 연사 타이머를 시작해야 함)
	AICharacter->StartFire();

	// 2. 경과 시간 초기화
	AttackTimeElapsed = 0.0f;

	// 3. Task를 InProgress 상태로 설정하여 TickTask를 받을 준비
	return EBTNodeResult::InProgress;
}

void UBTT_FireWeapon::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AttackTimeElapsed += DeltaSeconds;

	AAIController* AIController = OwnerComp.GetAIOwner();
	ALostSectorCharacter* AICharacter = AIController ? Cast<ALostSectorCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIController || !AICharacter || !BlackboardComp)
	{
		// 유효하지 않으면 실패로 Task 종료
		if (AttackTimeElapsed >= AttackDuration) FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// [핵심] 블랙보드에서 Player Vector Location을 가져옵니다.
	FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);

	if (TargetLocation != FVector::ZeroVector)
	{
		FVector StartLocation = AICharacter->GetActorLocation();

		FVector DirectionToTarget = TargetLocation - StartLocation;
		DirectionToTarget.Z = 0.0f; // 수평 회전만

		FRotator TargetRotation = DirectionToTarget.Rotation();
		FRotator CurrentRotation = AICharacter->GetActorRotation();
		float RotationSpeed = 10.0f;

		// 부드러운 회전 적용
		FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation, TargetRotation, DeltaSeconds, RotationSpeed
		);

		AICharacter->SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
	}


	// 지속 시간 초과 시 발사 중지 및 Task 종료
	if (AttackTimeElapsed >= AttackDuration)
	{
		AICharacter->StopFire(); // 연사 타이머 중지
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
