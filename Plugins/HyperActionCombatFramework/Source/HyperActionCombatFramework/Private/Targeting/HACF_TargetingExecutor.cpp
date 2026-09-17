// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/HACF_TargetingExecutor.h"
#include "Targeting/HACF_TargetingConfig.h"
#include "Targeting/HACF_TargetingTask.h"

FHACF_TargetingResult UHACF_TargetingExecutor::Execute(const FHACF_TargetingRequest& TargetingRequest) const
{
	FHACF_TargetingExecutionContext Context;
	if (!Context.Initialize(TargetingRequest))
	{
		FHACF_TargetingResult Result;
		Result.Reset(EHACF_TargetingResultStatus::Failed);
		return Result;
	}
	
	const UHACF_TargetingConfig* Config = TargetingRequest.Config;

	// Selection Tasks 실행
	if (!ExecuteTasks(Config->SelectionTasks, EHACF_TargetingTaskStage::Selection, Context))
	{
		FHACF_TargetingResult Result;
		Result.Reset(Context.FailureStatus);
		return Result;
	}

	if (Context.Candidates.IsEmpty())
	{
		FHACF_TargetingResult Result;
		Result.Reset(EHACF_TargetingResultStatus::NoCandidates);
		return Result;
	}
	
	// Filter Tasks 실행
	if (!ExecuteTasks(Config->FilterTasks, EHACF_TargetingTaskStage::Filter, Context))
	{
		FHACF_TargetingResult Result;
		Result.Reset(Context.FailureStatus);
		return Result;
	}
	
	if (Context.Candidates.IsEmpty())
	{
		FHACF_TargetingResult Result;
		Result.Reset(EHACF_TargetingResultStatus::NoCandidates);
		return Result;
	}

	// Score Tasks 실행
	if (!ExecuteTasks(Config->ScoreTasks, EHACF_TargetingTaskStage::Score, Context))
	{
		FHACF_TargetingResult Result;
		Result.Reset(Context.FailureStatus);
		return Result;
	}
	
	return BuildTargetingResult(Context);
}

bool UHACF_TargetingExecutor::ExecuteTasks(const TArray<TObjectPtr<UHACF_TargetingTask>>& Tasks,
	EHACF_TargetingTaskStage Stage, FHACF_TargetingExecutionContext& Context) const
{
	Context.CurrentStage = Stage;

	for (UHACF_TargetingTask* Task : Tasks)
	{
		Context.CurrentTaskIndex++;
		
		if (!IsValid(Task))
		{
			Context.Fail(EHACF_TargetingResultStatus::InvalidRequest, TEXT("Targeting config contains an invalid task."));
			return false;
		}
		
		if (Context.bCancelled)
		{
			Context.Fail(EHACF_TargetingResultStatus::Cancelled, TEXT("Targeting execution was cancelled."));
			return false;
		}

		switch (Task->Execute(Context))
		{
		case EHACF_TargetingTaskStatus::Success:
		case EHACF_TargetingTaskStatus::Skipped:
			break;
			
		case EHACF_TargetingTaskStatus::Failed:
		default:
			return false;
		}
	}
	
	Context.CurrentTaskIndex = INDEX_NONE;
	return true;
}

FHACF_TargetingResult UHACF_TargetingExecutor::BuildTargetingResult(const FHACF_TargetingExecutionContext& Context) const
{
	FHACF_TargetingResult Result;
	Result.Reset(EHACF_TargetingResultStatus::NoCandidates);
	
	const UHACF_TargetingConfig* Config = Context.Request.Config;
	if (!IsValid(Config))
	{
		Result.Reset(EHACF_TargetingResultStatus::InvalidRequest);
		return Result;
	}
	
	// 잘못된 후보가 최종 결과에 들어가지 않도록 정리한다.
	Result.Candidates.Reserve(Context.Candidates.Num());
	
	for (const FHACF_TargetingCandidate& Candidate : Context.Candidates)
	{
		if (!Candidate.IsValid())
		{
			continue;
		}

		if (Candidate.TargetCategory != Config->TargetCategory)
		{
			continue;
		}

		Result.Candidates.Add(Candidate);
	}

	if (Result.Candidates.IsEmpty())
	{
		return Result;
	}

	// 점수와 StableSortIndex를 이용해 결정적으로 정렬한다.
	const EHACF_TargetSortMode SortMode = Config->SortMode;

	Result.Candidates.Sort(
		[SortMode](const FHACF_TargetingCandidate& Left, const FHACF_TargetingCandidate& Right)
		{
			if (Left.Score != Right.Score)
			{
				return SortMode == EHACF_TargetSortMode::ScoreDescending
					? Left.Score > Right.Score
					: Left.Score < Right.Score;
			}

			return Left.StableSortIndex < Right.StableSortIndex;
		});

	// 요청 override가 반영된 최종 제한값을 사용한다.
	if (Result.Candidates.Num() > Context.EffectiveMaxTargets)
	{
		Result.Candidates.SetNum(Context.EffectiveMaxTargets, EAllowShrinking::No);
	}

	Result.Status = EHACF_TargetingResultStatus::Success;
	
	const FHACF_TargetingCandidate& PrimaryCandidate = Result.Candidates[0];

	Result.bHasTargetLocation = true;
	Result.TargetLocation = PrimaryCandidate.TargetLocation;

	Result.bHasTargetDirection = PrimaryCandidate.bHasDirection;
	Result.TargetDirection = PrimaryCandidate.bHasDirection ? PrimaryCandidate.TargetDirection : FVector::ZeroVector;

	return Result;
}
