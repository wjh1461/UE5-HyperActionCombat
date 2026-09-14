// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HACF_TargetingService.h"
#include "Components/ActorComponent.h"
#include "HACF_TargetingComponent.generated.h"

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
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
