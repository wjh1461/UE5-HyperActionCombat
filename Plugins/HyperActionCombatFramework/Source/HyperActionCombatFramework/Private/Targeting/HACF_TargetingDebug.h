#pragma once

#include "CoreMinimal.h"

class UWorld;
struct FHACF_TargetingRequest;
struct FHACF_TargetingResult;

namespace HACFTargetingDebug
{
	void DrawQuerySphere(const UWorld* World, const FVector& Center, float Radius);
	void DrawResult(const FHACF_TargetingRequest& Request, const FHACF_TargetingResult& Result);
}
