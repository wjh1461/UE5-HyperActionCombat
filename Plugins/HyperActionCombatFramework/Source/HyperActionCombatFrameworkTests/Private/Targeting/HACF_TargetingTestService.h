#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/HACF_CombatComponent.h"
#include "Targeting/HACF_TargetingService.h"
#include "HACF_TargetingTestService.generated.h"

/** Minimal observable targeting service used by CombatComponent integration tests. */
UCLASS()
class HYPERACTIONCOMBATFRAMEWORKTESTS_API UHACF_TargetingTestService : public UActorComponent, public IHACF_TargetingService
{
	GENERATED_BODY()

public:
	virtual FHACF_TargetingResult ExecuteTargeting_Implementation(const FHACF_TargetingRequest& TargetingRequest) override;

	FHACF_TargetingResult ResultToReturn;
	FHACF_TargetingRequest LastRequest;
	int32 ExecuteCount = 0;
};

/** Exposes the protected lifecycle entry point required by the BeginPlay cache test. */
UCLASS()
class HYPERACTIONCOMBATFRAMEWORKTESTS_API UHACF_TargetingTestCombatComponent : public UHACF_CombatComponent
{
	GENERATED_BODY()

public:
	void InvokeBeginPlay() { BeginPlay(); }
};
