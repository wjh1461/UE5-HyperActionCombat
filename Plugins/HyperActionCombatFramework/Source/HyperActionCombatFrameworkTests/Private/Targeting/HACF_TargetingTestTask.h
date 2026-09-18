#pragma once

#include "CoreMinimal.h"
#include "Targeting/HACF_TargetingTask.h"
#include "HACF_TargetingTestTask.generated.h"

/**
 * 타게팅 파이프라인 테스트에서 후보 생성, 실패, 취소를 재현하기 위한 설정형 Task.
 * 런타임 Task와 마찬가지로 요청별 상태는 Context에만 기록하고, 관찰값만 테스트 객체에 남긴다.
 */
UCLASS()
class HYPERACTIONCOMBATFRAMEWORKTESTS_API UHACF_TargetingTestTask : public UHACF_TargetingTask
{
	GENERATED_BODY()

public:
	virtual EHACF_TargetingTaskStatus Execute(FHACF_TargetingExecutionContext& Context) const override;

	TArray<FHACF_TargetingCandidate> CandidatesToAdd;
	EHACF_TargetingTaskStatus ReturnStatus = EHACF_TargetingTaskStatus::Success;
	EHACF_TargetingResultStatus FailureStatus = EHACF_TargetingResultStatus::Failed;
	bool bResetCandidates = false;
	bool bCancelContext = false;
	bool bFailContext = false;
	bool bAllocateStableSortIndex = true;

	// 여러 Task의 전체 실행 순서를 확인할 때 사용하는 선택적 기록 대상이다.
	int32 ExecutionToken = INDEX_NONE;
	TArray<int32>* ExecutionOrder = nullptr;

	mutable int32 ExecuteCount = 0;
	mutable TArray<EHACF_TargetingTaskStage> ObservedStages;
	mutable TArray<int32> ObservedTaskIndices;
};
