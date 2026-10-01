// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/HACF_TargetingExecutor.h"
#include "Targeting/HACF_TargetingConfig.h"
#include "Targeting/HACF_TargetingDebug.h"
#include "Targeting/HACF_TargetingProcessor.h"

namespace
{
	template <typename ProcessorType, typename ExecuteType>
	bool ExecuteProcessors(const TArray<TObjectPtr<ProcessorType>>& Processors, const EHACF_TargetingPhase Phase,
		FHACF_TargetingExecutionContext& Context, ExecuteType&& Execute)
	{
		Context.CurrentPhase = Phase;

		for (ProcessorType* Processor : Processors)
		{
			Context.CurrentProcessorIndex++;

			if (!IsValid(Processor))
			{
				Context.Fail(EHACF_TargetingResultStatus::InvalidRequest,
					TEXT("Targeting config contains an invalid processor."));
				return false;
			}

			if (Context.bCancelled)
			{
				Context.Fail(EHACF_TargetingResultStatus::Cancelled, TEXT("Targeting execution was cancelled."));
				return false;
			}

			switch (Execute(*Processor, Context))
			{
			case EHACF_TargetingProcessResult::Success:
			case EHACF_TargetingProcessResult::Skipped:
				break;

			case EHACF_TargetingProcessResult::Failed:
			default:
				return false;
			}
		}

		Context.CurrentProcessorIndex = INDEX_NONE;
		return true;
	}
}

FHACF_TargetingResult UHACF_TargetingExecutor::Execute(const FHACF_TargetingRequest& TargetingRequest) const
{
	FHACF_TargetingExecutionContext Context;
	if (!Context.Initialize(TargetingRequest))
	{
		FHACF_TargetingResult Result;
		Result.Reset(Context.FailureStatus);
		return Result;
	}

	const UHACF_TargetingConfig* Config = TargetingRequest.Config;

	// Selector 실행
	if (!ExecuteSelectors(Config->Selectors, Context))
	{
		FHACF_TargetingResult Result;
		Result.Reset(Context.FailureStatus);
		return Result;
	}

	NormalizeCandidates(Context.Candidates, Context);

	if (Context.Candidates.IsEmpty())
	{
		FHACF_TargetingResult Result;
		Result.Reset(EHACF_TargetingResultStatus::NoCandidates);
		return Result;
	}

	// Filter 실행
	if (!ExecuteFilters(Config->Filters, Context))
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

	// Scorer 실행
	if (!ExecuteScorers(Config->Scorers, Context))
	{
		FHACF_TargetingResult Result;
		Result.Reset(Context.FailureStatus);
		return Result;
	}

	FHACF_TargetingResult Result = BuildTargetingResult(Context);
	HACFTargetingDebug::DrawResult(TargetingRequest, Result);
	return Result;
}

bool UHACF_TargetingExecutor::ExecuteSelectors(const TArray<TObjectPtr<UHACF_TargetSelector>>& Selectors,
	FHACF_TargetingExecutionContext& Context) const
{
	return ExecuteProcessors(Selectors, EHACF_TargetingPhase::Selection, Context,
		[](const UHACF_TargetSelector& Selector, FHACF_TargetingExecutionContext& ExecutionContext)
		{
			return Selector.Select(ExecutionContext);
		});
}

bool UHACF_TargetingExecutor::ExecuteFilters(const TArray<TObjectPtr<UHACF_TargetFilter>>& Filters,
	FHACF_TargetingExecutionContext& Context) const
{
	return ExecuteProcessors(Filters, EHACF_TargetingPhase::Filter, Context,
		[](const UHACF_TargetFilter& Filter, FHACF_TargetingExecutionContext& ExecutionContext)
		{
			return Filter.Filter(ExecutionContext);
		});
}

bool UHACF_TargetingExecutor::ExecuteScorers(const TArray<TObjectPtr<UHACF_TargetScorer>>& Scorers,
	FHACF_TargetingExecutionContext& Context) const
{
	return ExecuteProcessors(Scorers, EHACF_TargetingPhase::Score, Context,
		[](const UHACF_TargetScorer& Scorer, FHACF_TargetingExecutionContext& ExecutionContext)
		{
			return Scorer.Score(ExecutionContext);
		});
}

void UHACF_TargetingExecutor::NormalizeCandidates(TArray<FHACF_TargetingCandidate>& Candidates,
	const FHACF_TargetingExecutionContext& Context) const
{
	const UHACF_TargetingConfig* Config = Context.Request.Config.Get();
	if (!IsValid(Config))
	{
		Candidates.Reset();
		return;
	}

	TSet<const AActor*> ExcludedActors;
	for (const TWeakObjectPtr<AActor>& Actor : Context.Request.ExcludedActors)
	{
		if (Actor.IsValid())
		{
			ExcludedActors.Add(Actor.Get());
		}
	}
	if (Config->bExcludeSourceActor && Context.Request.SourceActor.IsValid())
	{
		ExcludedActors.Add(Context.Request.SourceActor.Get());
	}

	TSet<const AActor*> SeenActors;
	// RemoveAll은 남은 후보의 순서를 유지하며, 부가 정보와 정렬 키를 변경하지 않는다.
	Candidates.RemoveAll([Config, &ExcludedActors, &SeenActors](const FHACF_TargetingCandidate& Candidate)
	{
		if (!Candidate.IsValid() || Candidate.TargetCategory != Config->TargetCategory)
		{
			return true;
		}
		if (Candidate.TargetCategory == EHACF_TargetCategory::Actor)
		{
			const AActor* Actor = Candidate.TargetActor.Get();
			if (ExcludedActors.Contains(Actor) || SeenActors.Contains(Actor))
			{
				return true;
			}
			SeenActors.Add(Actor);
		}
		return false;
	});
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

	// 커스텀 Processor 실행 후 결과를 정렬하고 개수를 제한하기 전에 공통 규칙을 다시 적용한다.
	Result.Candidates = Context.Candidates;
	NormalizeCandidates(Result.Candidates, Context);

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
