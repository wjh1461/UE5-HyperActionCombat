// Fill out your copyright notice in the Description page of Project Settings.


#include "TestGameplayAbility.h"
#include "Core/HACF_CombatComponent.h"

void UTestGameplayAbility::EndTestAbility()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UTestGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	++ActivationCount;
}

void UTestGameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	++InputPressedCount;
}

void UTestGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	++InputReleasedCount;
}

bool UTestCannotActivateGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return false;
}

void UTestOpensActionWindowOnActivationGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UHACF_CombatComponent* CombatComponent = GetCombatComponent();
	if (!CombatComponent)
	{
		return;
	}

	FHACF_ActionWindowDefinition Definition;
	Definition.AcceptedInputTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag")));
	CombatComponent->OpenActionWindow(Definition, this,
		FHACF_ActionWindowInputHandler::CreateUObject(
			this, &UTestOpensActionWindowOnActivationGameplayAbility::HandleWindowInput));
	bSynchronousProcessSucceeded = CombatComponent->ProcessBufferedInput();
}

bool UTestOpensActionWindowOnActivationGameplayAbility::HandleWindowInput(
	const FHACF_ActionWindowInputContext& Context)
{
	++WindowDispatchCount;
	return true;
}

void UTestQueuesSecondaryInputOnActivationGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UHACF_CombatComponent* CombatComponent = GetCombatComponent())
	{
		CombatComponent->AbilityInputTagPressed(
			FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Secondary")));
	}
}
