// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/HACF_AbilitySet.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbilityBase.h"

void FHACF_AbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FHACF_AbilitySet_GrantedHandles::TakeFromAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);
	
	if (!ASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	for (FGameplayAbilitySpecHandle AbilitySpecHandle : AbilitySpecHandles)
	{
		if (AbilitySpecHandle.IsValid())
		{
			ASC->ClearAbility(AbilitySpecHandle);
		}
	}
	
	AbilitySpecHandles.Reset();
}

void UHACF_AbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, 
                                           FHACF_AbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	for (int AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FHACF_AbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];
		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogTemp, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}
		
		UHACF_GameplayAbilityBase* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UHACF_GameplayAbilityBase>();
		
		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);
		
		const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}
}
