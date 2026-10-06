#include "Targeting/HACF_TargetingTestService.h"

FHACF_TargetingResult UHACF_TargetingTestService::ExecuteTargeting_Implementation(
	const FHACF_TargetingRequest& TargetingRequest)
{
	++ExecuteCount;
	LastRequest = TargetingRequest;
	return ResultToReturn;
}
