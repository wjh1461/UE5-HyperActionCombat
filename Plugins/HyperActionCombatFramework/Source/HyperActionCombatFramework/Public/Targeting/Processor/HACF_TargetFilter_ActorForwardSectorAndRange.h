// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Targeting/HACF_TargetingProcessor.h"
#include "HACF_TargetFilter_ActorForwardSectorAndRange.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "Actor Forward Sector And Range"))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetFilter_ActorForwardSectorAndRange : public UHACF_TargetFilter
{
	GENERATED_BODY()
	
public:
	virtual EHACF_TargetingProcessResult Filter(FHACF_TargetingExecutionContext& Context) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float HalfAngleDegrees = 45.0f;
};
