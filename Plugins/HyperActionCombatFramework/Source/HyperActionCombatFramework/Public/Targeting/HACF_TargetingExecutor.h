// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HACF_TargetingTypes.h"
#include "HACF_TargetingExecutor.generated.h"

class UHACF_TargetingTask;

/**
 * 
 */
UCLASS()
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetingExecutor : public UObject
{
	GENERATED_BODY()
	
public:
	FHACF_TargetingResult Execute(const FHACF_TargetingRequest& TargetingRequest) const;
	
private:
	bool ExecuteTasks(const TArray<TObjectPtr<UHACF_TargetingTask>>& Tasks, EHACF_TargetingTaskStage Stage, FHACF_TargetingExecutionContext& Context) const;
	
	FHACF_TargetingResult BuildTargetingResult(const FHACF_TargetingExecutionContext& Context) const;
	
};
