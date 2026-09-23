// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/Processor/HACF_TargetFilter_ActorExclusionAndRange.h"
#include "Targeting/HACF_TargetingConfig.h"

EHACF_TargetingProcessResult UHACF_TargetFilter_ActorExclusionAndRange::Filter(FHACF_TargetingExecutionContext& Context) const
{
	const UHACF_TargetingConfig* Config = Context.Request.Config.Get();
	if (!IsValid(Config) || Config->TargetCategory != EHACF_TargetCategory::Actor)
	{
		Context.Fail(EHACF_TargetingResultStatus::InvalidRequest,
			TEXT("Actor exclusion and range filter requires an actor target config."));
		return EHACF_TargetingProcessResult::Failed;
	}

	TSet<const AActor*> ExcludedActors;
	ExcludedActors.Reserve(Context.Request.ExcludedActors.Num() + 1);

	for (const TWeakObjectPtr<AActor>& ExcludedActor : Context.Request.ExcludedActors)
	{
		if (ExcludedActor.IsValid())
		{
			ExcludedActors.Add(ExcludedActor.Get());
		}
	}

	if (Config->bExcludeSourceActor && Context.Request.SourceActor.IsValid())
	{
		ExcludedActors.Add(Context.Request.SourceActor.Get());
	}

	const FVector RangeOrigin = Context.Request.Origin;
	const double RangeSquared = FMath::Square(static_cast<double>(Context.EffectiveRange));

	Context.Candidates.RemoveAll(
		[&ExcludedActors, RangeOrigin, RangeSquared](const FHACF_TargetingCandidate& Candidate)
		{
			if (!Candidate.IsValid() || Candidate.TargetCategory != EHACF_TargetCategory::Actor)
			{
				return true;
			}

			if (ExcludedActors.Contains(Candidate.TargetActor.Get()))
			{
				return true;
			}

			return FVector::DistSquared(RangeOrigin, Candidate.TargetLocation) > RangeSquared;
		});

	for (const FHACF_TargetingCandidate& Candidate : Context.Candidates)
	{
		UE_LOG(LogTemp, Log, TEXT("Filtered Candidate: %s"), *Candidate.TargetActor.Get()->GetName());
	}

	return EHACF_TargetingProcessResult::Success;
}
