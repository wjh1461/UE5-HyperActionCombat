// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HACF_TargetingTypes.h"
#include "UObject/Object.h"
#include "HACF_TargetingProcessor.generated.h"

/** Common base for synchronous processors in the targeting pipeline. */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetingProcessor : public UObject
{
	GENERATED_BODY()
};

/** Adds candidates to the targeting execution context. */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetSelector : public UHACF_TargetingProcessor
{
	GENERATED_BODY()

public:
	virtual EHACF_TargetingProcessResult Select(FHACF_TargetingExecutionContext& Context) const
		PURE_VIRTUAL(UHACF_TargetSelector::Select, return EHACF_TargetingProcessResult::Failed;);
};

/** Removes candidates that do not satisfy a targeting condition. */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetFilter : public UHACF_TargetingProcessor
{
	GENERATED_BODY()

public:
	virtual EHACF_TargetingProcessResult Filter(FHACF_TargetingExecutionContext& Context) const
		PURE_VIRTUAL(UHACF_TargetFilter::Filter, return EHACF_TargetingProcessResult::Failed;);
};

/** Evaluates and updates scores for the current targeting candidates. */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetScorer : public UHACF_TargetingProcessor
{
	GENERATED_BODY()

public:
	virtual EHACF_TargetingProcessResult Score(FHACF_TargetingExecutionContext& Context) const
		PURE_VIRTUAL(UHACF_TargetScorer::Score, return EHACF_TargetingProcessResult::Failed;);
};
