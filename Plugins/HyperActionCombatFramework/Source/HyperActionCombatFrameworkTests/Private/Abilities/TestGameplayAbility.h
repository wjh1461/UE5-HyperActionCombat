// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbilityBase.h"
#include "TestGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class HYPERACTIONCOMBATFRAMEWORKTESTS_API UTestGameplayAbility : public UHACF_GameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	void SetActivationPolicy(EHACFAbilityActivationPolicy activationPolicy) { ActivationPolicy = activationPolicy; }
};
