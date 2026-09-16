// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HACF_TargetingTypes.h"
#include "UObject/Interface.h"
#include "HACF_TargetingService.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UHACF_TargetingService : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HYPERACTIONCOMBATFRAMEWORK_API IHACF_TargetingService
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "HACF|Targeting")
	FHACF_TargetingResult ExecuteTargeting(const FHACF_TargetingRequest& TargetingRequest);
};
