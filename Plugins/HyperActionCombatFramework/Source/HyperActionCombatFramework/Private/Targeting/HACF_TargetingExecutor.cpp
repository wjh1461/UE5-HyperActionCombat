// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/HACF_TargetingExecutor.h"
#include "Targeting/HACF_TargetingConfig.h"

FHACF_TargetingResult UHACF_TargetingExecutor::Execute(const FHACF_TargetingRequest& TargetingRequest) const
{
	FHACF_TargetingResult Result;

	if (!IsValid(TargetingRequest.Config))
	{
		Result.Status = EHACF_TargetingResultStatus::InvalidRequest;
		return Result;
	}
	
	FHACF_TargetingExecutionContext Context;
	Context.Initialize(TargetingRequest);

	if (Context.Candidates.IsEmpty())
	{
		Result.Status = EHACF_TargetingResultStatus::NoCandidates;
		return Result;
	}
	
	return Result;
}

bool UHACF_TargetingExecutor::ExecuteTasks(const TArray<TObjectPtr<UHACF_TargetingTask>>& Tasks,
	EHACF_TargetingTaskStage Stage, FHACF_TargetingExecutionContext& Context) const
{
	return false;
}
