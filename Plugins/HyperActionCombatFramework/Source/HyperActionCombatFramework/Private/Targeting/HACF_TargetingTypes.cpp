// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/HACF_TargetingTypes.h"

#include "Targeting/HACF_TargetingConfig.h"

bool FHACF_TargetingCandidate::IsValid() const
{
	if (TargetLocation.ContainsNaN() || !FMath::IsFinite(Score))
	{
		return false;
	}

	if (bHasDirection && (TargetDirection.ContainsNaN() || TargetDirection.IsNearlyZero()))
	{
		return false;
	}

	if (bHasHitResult && HitResult.ImpactPoint.ContainsNaN())
	{
		return false;
	}

	switch (TargetCategory)
	{
		case EHACF_TargetCategory::Actor:
		return TargetActor.IsValid();

		case EHACF_TargetCategory::Location:
		return true;
		
		default:
		return false;
	}
}

bool FHACF_TargetingExecutionContext::Initialize(const FHACF_TargetingRequest& InRequest)
{
	Request = InRequest;
	Candidates.Reset();

	CurrentPhase = EHACF_TargetingPhase::Selection;
	CurrentProcessorIndex = INDEX_NONE;
	NextStableSortIndex = 0;
	bCancelled = false;
	FailureStatus = EHACF_TargetingResultStatus::Failed;

#if !UE_BUILD_SHIPPING
	FailureReason.Reset();
#endif

	if (!IsValid(Request.Config))
	{
		Fail(EHACF_TargetingResultStatus::InvalidRequest, TEXT("Targeting config is invalid."));
		return false;
	}

	EffectiveRange = Request.bOverrideRange ? Request.RangeOverride : Request.Config->DefaultRange;

	EffectiveMaxTargets = Request.bOverrideMaxTargets ? Request.MaxTargetsOverride : Request.Config->MaxTargets;

	if (!FMath::IsFinite(EffectiveRange) || EffectiveRange <= 0.0f || EffectiveMaxTargets <= 0)
	{
		Fail(EHACF_TargetingResultStatus::InvalidRequest, TEXT("Effective targeting parameters are invalid."));
		return false;
	}

	return true;
}

void FHACF_TargetingExecutionContext::Fail(EHACF_TargetingResultStatus InStatus, const FString& InReason)
{
	FailureStatus = InStatus;
	Candidates.Reset();

#if !UE_BUILD_SHIPPING
	FailureReason = InReason;
#endif
}

void FHACF_TargetingResult::Reset(EHACF_TargetingResultStatus NewStatus)
{
	Status = NewStatus;
	Candidates.Reset();

	bHasTargetLocation = false;
	TargetLocation = FVector::ZeroVector;

	bHasTargetDirection = false;
	TargetDirection = FVector::ZeroVector;
}
