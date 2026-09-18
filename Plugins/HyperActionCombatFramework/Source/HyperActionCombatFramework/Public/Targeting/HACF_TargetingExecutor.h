// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HACF_TargetingTypes.h"
#include "HACF_TargetingExecutor.generated.h"

class UHACF_TargetFilter;
class UHACF_TargetScorer;
class UHACF_TargetSelector;

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
	bool ExecuteSelectors(const TArray<TObjectPtr<UHACF_TargetSelector>>& Selectors, FHACF_TargetingExecutionContext& Context) const;
	bool ExecuteFilters(const TArray<TObjectPtr<UHACF_TargetFilter>>& Filters, FHACF_TargetingExecutionContext& Context) const;
	bool ExecuteScorers(const TArray<TObjectPtr<UHACF_TargetScorer>>& Scorers, FHACF_TargetingExecutionContext& Context) const;
	
	FHACF_TargetingResult BuildTargetingResult(const FHACF_TargetingExecutionContext& Context) const;
	
};
