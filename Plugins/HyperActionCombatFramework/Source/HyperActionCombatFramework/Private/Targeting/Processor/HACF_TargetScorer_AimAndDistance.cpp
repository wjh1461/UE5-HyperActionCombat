// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/Processor/HACF_TargetScorer_AimAndDistance.h"

#include "Targeting/HACF_TargetingConfig.h"

namespace
{
	bool IsFiniteNonNegative(const double Value)
	{
		return FMath::IsFinite(Value) && Value >= 0.0;
	}
}

EHACF_TargetingProcessResult UHACF_TargetScorer_AimAndDistance::Score(FHACF_TargetingExecutionContext& Context) const
{
	const UHACF_TargetingConfig* Config = Context.Request.Config.Get();
	const bool bHasValidRequest = IsValid(Config) && !Context.Request.Origin.ContainsNaN() &&
		!Context.Request.AimDirection.ContainsNaN() && !Context.Request.AimDirection.IsNearlyZero() &&
		FMath::IsFinite(Context.EffectiveRange) && Context.EffectiveRange > 0.0f;
	if (!bHasValidRequest)
	{
		Context.Fail(EHACF_TargetingResultStatus::InvalidRequest, TEXT("Aim and distance scorer requires a valid origin, aim direction, and effective range."));
		return EHACF_TargetingProcessResult::Failed;
	}

	const double WeightSum = static_cast<double>(AimWeight) + static_cast<double>(DistanceWeight);
	const bool bHasValidScoringSettings = IsFiniteNonNegative(AimWeight) && IsFiniteNonNegative(DistanceWeight) &&
		IsFiniteNonNegative(PreferredTargetBonus) && FMath::IsFinite(WeightSum) && WeightSum > 0.0;
	if (!bHasValidScoringSettings)
	{
		Context.Fail(EHACF_TargetingResultStatus::InvalidRequest, TEXT("Aim and distance scorer requires non-negative scoring weights with a positive sum."));
		return EHACF_TargetingProcessResult::Failed;
	}

	const FVector AimDirection = Context.Request.AimDirection.GetSafeNormal();
	const FVector Origin = Context.Request.Origin;
	const double EffectiveRange = Context.EffectiveRange;

	for (FHACF_TargetingCandidate& Candidate : Context.Candidates)
	{
		if (!Candidate.IsValid() || Candidate.TargetCategory != Config->TargetCategory)
		{
			continue;
		}

		const FVector ToTarget = Candidate.TargetLocation - Origin;
		const double Distance = ToTarget.Size();
		
		const FVector DirectionToTarget = ToTarget.GetSafeNormal();
		const double AimAlignment = FMath::Clamp(FVector::DotProduct(AimDirection, DirectionToTarget), -1.0, 1.0);
		
		const double AimScore = (AimAlignment + 1.0) * 0.5;
		const double DistanceScore = 1.0 - FMath::Clamp(Distance / EffectiveRange, 0.0, 1.0);
		const double WeightedScore = (AimScore * static_cast<double>(AimWeight) + DistanceScore * static_cast<double>(DistanceWeight)) / WeightSum;
		const double AppliedPreferredTargetBonus = Candidate.TargetCategory == EHACF_TargetCategory::Actor && 
			Candidate.TargetActor == Context.Request.PreferredTarget ? static_cast<double>(PreferredTargetBonus) : 0.0;
		
		const double NewScore = static_cast<double>(Candidate.Score) + WeightedScore + AppliedPreferredTargetBonus;
		if (!FMath::IsFinite(NewScore) || NewScore > MAX_flt || NewScore < -MAX_flt)
		{
			Context.Fail(EHACF_TargetingResultStatus::Failed, TEXT("Aim and distance scorer produced a non-finite candidate score."));
			return EHACF_TargetingProcessResult::Failed;
		}

		Candidate.Score = static_cast<float>(NewScore);
	}

	return EHACF_TargetingProcessResult::Success;
}
