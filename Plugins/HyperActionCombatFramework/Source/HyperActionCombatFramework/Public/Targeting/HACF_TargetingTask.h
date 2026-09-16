// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HACF_TargetingTypes.h"
#include "UObject/Object.h"
#include "HACF_TargetingTask.generated.h"

/**
 * 
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetingTask : public UObject
{
	GENERATED_BODY()
	
public:
	virtual EHACF_TargetingTaskStatus Execute(FHACF_TargetingExecutionContext& Context) const PURE_VIRTUAL(UHACF_TargetingTask::Execute, return EHACF_TargetingTaskStatus::Failed;);
};
