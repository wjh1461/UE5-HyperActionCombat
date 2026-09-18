#pragma once

#include "CoreMinimal.h"
#include "Targeting/HACF_TargetingProcessor.h"
#include "HACF_TargetingTestProcessors.generated.h"

/** Shared configurable behavior for role-specific targeting processor test doubles. */
struct FHACF_TargetingTestProcessorBehavior
{
	EHACF_TargetingProcessResult Execute(FHACF_TargetingExecutionContext& Context) const;

	TArray<FHACF_TargetingCandidate> CandidatesToAdd;
	EHACF_TargetingProcessResult ReturnResult = EHACF_TargetingProcessResult::Success;
	EHACF_TargetingResultStatus FailureStatus = EHACF_TargetingResultStatus::Failed;
	bool bResetCandidates = false;
	bool bCancelContext = false;
	bool bFailContext = false;
	bool bAllocateStableSortIndex = true;

	int32 ExecutionToken = INDEX_NONE;
	TArray<int32>* ExecutionOrder = nullptr;

	mutable int32 ExecuteCount = 0;
	mutable TArray<EHACF_TargetingPhase> ObservedPhases;
	mutable TArray<int32> ObservedProcessorIndices;
};

UCLASS()
class HYPERACTIONCOMBATFRAMEWORKTESTS_API UHACF_TargetingTestSelector : public UHACF_TargetSelector
{
	GENERATED_BODY()

public:
	virtual EHACF_TargetingProcessResult Select(FHACF_TargetingExecutionContext& Context) const override;

	FHACF_TargetingTestProcessorBehavior Behavior;
};

UCLASS()
class HYPERACTIONCOMBATFRAMEWORKTESTS_API UHACF_TargetingTestFilter : public UHACF_TargetFilter
{
	GENERATED_BODY()

public:
	virtual EHACF_TargetingProcessResult Filter(FHACF_TargetingExecutionContext& Context) const override;

	FHACF_TargetingTestProcessorBehavior Behavior;
};

UCLASS()
class HYPERACTIONCOMBATFRAMEWORKTESTS_API UHACF_TargetingTestScorer : public UHACF_TargetScorer
{
	GENERATED_BODY()

public:
	virtual EHACF_TargetingProcessResult Score(FHACF_TargetingExecutionContext& Context) const override;

	FHACF_TargetingTestProcessorBehavior Behavior;
};
