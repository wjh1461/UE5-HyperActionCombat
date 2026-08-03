// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbility.h"
#include "TestGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class HYPERACTIONCOMBATFRAMEWORKTESTS_API UTestGameplayAbility : public UHACF_GameplayAbility
{
	GENERATED_BODY()
	
public:
	void SetActivationPolicy(EHACFAbilityActivationPolicy activationPolicy) { ActivationPolicy = activationPolicy; }
	void EndTestAbility();
	int32 GetActivationCount() const { return ActivationCount; }
	int32 GetInputPressedCount() const { return InputPressedCount; }
	int32 GetInputReleasedCount() const { return InputReleasedCount; }

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

private:
	int32 ActivationCount = 0;
	int32 InputPressedCount = 0;
	int32 InputReleasedCount = 0;
};

UCLASS()
class HYPERACTIONCOMBATFRAMEWORKTESTS_API UTestWhileInputActiveGameplayAbility : public UTestGameplayAbility
{
	GENERATED_BODY()

public:
	UTestWhileInputActiveGameplayAbility()
	{
		SetActivationPolicy(EHACFAbilityActivationPolicy::WhileInputActive);
	}
};
