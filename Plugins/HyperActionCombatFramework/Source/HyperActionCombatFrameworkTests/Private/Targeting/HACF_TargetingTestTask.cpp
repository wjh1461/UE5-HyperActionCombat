#include "Targeting/HACF_TargetingTestTask.h"

EHACF_TargetingTaskStatus UHACF_TargetingTestTask::Execute(FHACF_TargetingExecutionContext& Context) const
{
	++ExecuteCount;
	ObservedStages.Add(Context.CurrentStage);
	ObservedTaskIndices.Add(Context.CurrentTaskIndex);

	if (ExecutionOrder && ExecutionToken != INDEX_NONE)
	{
		ExecutionOrder->Add(ExecutionToken);
	}

	if (bResetCandidates)
	{
		Context.Candidates.Reset();
	}

	for (FHACF_TargetingCandidate Candidate : CandidatesToAdd)
	{
		// 실제 Selection Task처럼 요청 Context에서 결정적 정렬 키를 발급받는다.
		if (bAllocateStableSortIndex)
		{
			Candidate.StableSortIndex = Context.AllocateStableSortIndex();
		}

		Context.Candidates.Add(MoveTemp(Candidate));
	}

	if (bCancelContext)
	{
		Context.bCancelled = true;
	}

	if (bFailContext)
	{
		Context.Fail(FailureStatus, TEXT("Targeting test task requested failure."));
	}

	return ReturnStatus;
}
