// Fill out your copyright notice in the Description page of Project Settings.


#include "Targeting/HACF_TargetingComponent.h"
#include "Targeting/HACF_TargetingExecutor.h"


// Sets default values for this component's properties
UHACF_TargetingComponent::UHACF_TargetingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHACF_TargetingComponent::BeginPlay()
{
	Super::BeginPlay();
	
	TargetingExecutor = NewObject<UHACF_TargetingExecutor>();
}

FHACF_TargetingResult UHACF_TargetingComponent::ExecuteTargeting_Implementation(const FHACF_TargetingRequest& TargetingRequest)
{
	if (!IsValid(TargetingExecutor))
	{
		FHACF_TargetingResult Result;
		Result.Reset(EHACF_TargetingResultStatus::NotReady);
		
		return Result;
	}
	
	return TargetingExecutor->Execute(TargetingRequest);
}

