// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/Processor/HACF_TargetSelector_SphereOverlap.h"
#include "Targeting/HACF_TargetingConfig.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UHACF_TargetSelector_SphereOverlap::UHACF_TargetSelector_SphereOverlap()
{
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
}

EHACF_TargetingProcessResult UHACF_TargetSelector_SphereOverlap::Select(FHACF_TargetingExecutionContext& Context) const
{
	AActor* SourceActor = Context.Request.SourceActor.Get();
	const UWorld* World = IsValid(SourceActor) ? SourceActor->GetWorld() : nullptr;
	if (!IsValid(Context.Request.Config) || Context.Request.Config->TargetCategory != EHACF_TargetCategory::Actor ||
		!IsValid(World) || ObjectTypes.IsEmpty())
	{
		Context.Fail(EHACF_TargetingResultStatus::InvalidRequest,
			TEXT("Sphere overlap requires an actor target config, a source world, and object types."));
		return EHACF_TargetingProcessResult::Failed;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	for (const TEnumAsByte<EObjectTypeQuery> ObjectType : ObjectTypes)
	{
		const ECollisionChannel Channel = UEngineTypes::ConvertToCollisionChannel(ObjectType.GetValue());
		if (!FCollisionObjectQueryParams::IsValidObjectQuery(Channel))
		{
			Context.Fail(EHACF_TargetingResultStatus::InvalidRequest, TEXT("Sphere overlap has an invalid object type."));
			return EHACF_TargetingProcessResult::Failed;
		}
		ObjectQueryParams.AddObjectTypesToQuery(Channel);
	}

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, SourceActor->GetActorLocation(), FQuat::Identity, ObjectQueryParams,
		FCollisionShape::MakeSphere(Context.EffectiveRange));

	TSet<AActor*> SeenActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!IsValid(Actor) || SeenActors.Contains(Actor))
		{
			continue;
		}

		SeenActors.Add(Actor);

		FHACF_TargetingCandidate& Candidate = Context.Candidates.AddDefaulted_GetRef();
		Candidate.TargetCategory = EHACF_TargetCategory::Actor;
		Candidate.TargetActor = Actor;
		Candidate.TargetLocation = Actor->GetActorLocation();
		Candidate.StableSortIndex = Context.AllocateStableSortIndex();
	}

	return EHACF_TargetingProcessResult::Success;
}
