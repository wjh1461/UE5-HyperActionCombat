// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HACF_TargetingService.h"
#include "Components/ActorComponent.h"
#include "HACF_TargetingComponent.generated.h"

class UHACF_TargetingExecutor;

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetingComponent : public UActorComponent, public IHACF_TargetingService
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHACF_TargetingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual FHACF_TargetingResult ExecuteTargeting_Implementation(const FHACF_TargetingRequest& TargetingRequest) override;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UHACF_TargetingExecutor> TargetingExecutor;
};
