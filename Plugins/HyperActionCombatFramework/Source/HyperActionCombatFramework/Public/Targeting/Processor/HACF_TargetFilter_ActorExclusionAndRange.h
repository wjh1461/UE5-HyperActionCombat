// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Targeting/HACF_TargetingProcessor.h"
#include "HACF_TargetFilter_ActorExclusionAndRange.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Actor Exclusion And Range Target Filter"))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetFilter_ActorExclusionAndRange : public UHACF_TargetFilter
{
	GENERATED_BODY()
	
public:
	virtual EHACF_TargetingProcessResult Filter(FHACF_TargetingExecutionContext& Context) const override;
};
