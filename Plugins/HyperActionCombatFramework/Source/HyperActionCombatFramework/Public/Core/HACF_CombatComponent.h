// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HACF_CombatComponent.generated.h"

/*
 *
 */
UCLASS( ClassGroup=(HyperActionCombatFramework), meta=(BlueprintSpawnableComponent) )
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_CombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHACF_CombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
