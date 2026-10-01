#include "Targeting/HACF_TargetingTestProcessors.h"

EHACF_TargetingProcessResult FHACF_TargetingTestProcessorBehavior::Execute(
	FHACF_TargetingExecutionContext& Context) const
{
	++ExecuteCount;
	ObservedCandidates = Context.Candidates;
	ObservedPhases.Add(Context.CurrentPhase);
	ObservedProcessorIndices.Add(Context.CurrentProcessorIndex);

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
		Context.Fail(FailureStatus, TEXT("Targeting test processor requested failure."));
	}

	return ReturnResult;
}

EHACF_TargetingProcessResult UHACF_TargetingTestSelector::Select(FHACF_TargetingExecutionContext& Context) const
{
	return Behavior.Execute(Context);
}

EHACF_TargetingProcessResult UHACF_TargetingTestFilter::Filter(FHACF_TargetingExecutionContext& Context) const
{
	return Behavior.Execute(Context);
}

EHACF_TargetingProcessResult UHACF_TargetingTestScorer::Score(FHACF_TargetingExecutionContext& Context) const
{
	return Behavior.Execute(Context);
}
