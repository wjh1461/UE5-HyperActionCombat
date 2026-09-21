// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Targeting/HACF_TargetingProcessor.h"
#include "HACF_TargetScorer_AimAndDistance.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "Aim And Distance Target Scorer"))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetScorer_AimAndDistance : public UHACF_TargetScorer
{
	GENERATED_BODY()
	
public:
	virtual EHACF_TargetingProcessResult Score(FHACF_TargetingExecutionContext& Context) const override;
};
