#include "Targeting/HACF_TargetingDebug.h"

#if !UE_BUILD_SHIPPING
#include "Targeting/HACF_TargetingTypes.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarHACFTargetingDebugDraw(TEXT("hacf.Targeting.DebugDraw"),
		0, TEXT("Draw targeting queries and results. 0: off, 1: on."), ECVF_Default);

	bool IsDebugDrawEnabled()
	{
		return CVarHACFTargetingDebugDraw.GetValueOnGameThread() != 0;
	}

	constexpr float DebugDrawDuration = 1.0f;
}
#endif

void HACFTargetingDebug::DrawQuerySphere(const UWorld* World, const FVector& Center, float Radius)
{
#if !UE_BUILD_SHIPPING
	if (IsDebugDrawEnabled() && IsValid(World))
	{
		DrawDebugSphere(World, Center, Radius, 24, FColor::Yellow, false, DebugDrawDuration);
	}
#endif
}

void HACFTargetingDebug::DrawResult(const FHACF_TargetingRequest& Request, const FHACF_TargetingResult& Result)
{
#if !UE_BUILD_SHIPPING
	if (!IsDebugDrawEnabled() || !Result.IsSuccess())
	{
		return;
	}

	const AActor* SourceActor = Request.SourceActor.Get();
	const UWorld* World = IsValid(SourceActor) ? SourceActor->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		return;
	}

	for (int32 Index = 0; Index < Result.Candidates.Num(); ++Index)
	{
		const FHACF_TargetingCandidate& Candidate = Result.Candidates[Index];
		const FColor Color = Index == 0 ? FColor::Green : FColor::Cyan;
		DrawDebugSphere(World, Candidate.TargetLocation, 18.0f, 8, Color, false, DebugDrawDuration);
	}
#endif
}
