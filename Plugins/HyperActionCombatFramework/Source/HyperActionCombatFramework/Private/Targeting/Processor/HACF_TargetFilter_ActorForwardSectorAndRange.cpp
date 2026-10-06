// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/Processor/HACF_TargetFilter_ActorForwardSectorAndRange.h"

#include "GameFramework/Actor.h"
#include "Targeting/HACF_TargetingConfig.h"

EHACF_TargetingProcessResult UHACF_TargetFilter_ActorForwardSectorAndRange::Filter(
	FHACF_TargetingExecutionContext& Context) const
{
	const UHACF_TargetingConfig* Config = Context.Request.Config.Get();
	const AActor* SourceActor = Context.Request.SourceActor.Get();
	
	const bool bHasValidActorRequest = IsValid(Config) && Config->TargetCategory == EHACF_TargetCategory::Actor
		&& IsValid(SourceActor) && !Context.Request.Origin.ContainsNaN();
	const bool bHasValidRange = FMath::IsFinite(Context.EffectiveRange) && Context.EffectiveRange > 0.0f;
	const bool bHasValidHalfAngle = FMath::IsWithinInclusive(HalfAngleDegrees, 0.0f, 180.0f);
	if (!bHasValidActorRequest || !bHasValidRange || !bHasValidHalfAngle)
	{
		Context.Fail(EHACF_TargetingResultStatus::InvalidRequest, TEXT("Actor forward sector and range filter received invalid parameters."));

		return EHACF_TargetingProcessResult::Failed;
	}

	FVector Forward2D = SourceActor->GetActorForwardVector();
	Forward2D.Z = 0.0;
	if (!Forward2D.Normalize())
	{
		Context.Fail(EHACF_TargetingResultStatus::InvalidRequest, TEXT("Actor forward sector and range filter requires a valid horizontal source actor forward direction."));
		return EHACF_TargetingProcessResult::Failed;
	}

	const FVector Origin = Context.Request.Origin;
	const double RangeSquared = FMath::Square(static_cast<double>(Context.EffectiveRange));
	const double MinimumDot = FMath::Cos(FMath::DegreesToRadians(static_cast<double>(HalfAngleDegrees)));

	Context.Candidates.RemoveAll(
		[Origin, Forward2D, RangeSquared, MinimumDot, SourceActor, Config](const FHACF_TargetingCandidate& Candidate)
		{
			if (!Candidate.IsValid() || Candidate.TargetCategory != EHACF_TargetCategory::Actor
				|| (Config->bExcludeSourceActor && Candidate.TargetActor.Get() == SourceActor))
			{
				return true;
			}

			const FVector ToTarget3D = Candidate.TargetLocation - Origin;
			if (ToTarget3D.SizeSquared() > RangeSquared)
			{
				return true;
			}

			FVector ToTarget2D = ToTarget3D;
			ToTarget2D.Z = 0.0;
			if (!ToTarget2D.Normalize())
			{
				return false;
			}

			return FVector::DotProduct(Forward2D, ToTarget2D) < MinimumDot;
		});

	return EHACF_TargetingProcessResult::Success;
}
