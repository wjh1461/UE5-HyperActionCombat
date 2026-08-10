// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/HACF_AbilitySet.h"
#include "AbilitySystemComponent.h"
#include "HACF_LogChannels.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbility.h"

void FHACF_AbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FHACF_AbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
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

	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles)
	{
		if (AbilitySpecHandle.IsValid())
		{
			ASC->ClearAbility(AbilitySpecHandle);
		}
	}
	
	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}
	
	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
}

void UHACF_AbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, 
                                           FHACF_AbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	for (int AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FHACF_AbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];
		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogHACF, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}
		
		UHACF_GameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UHACF_GameplayAbility>();
		
		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);
		
		const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}
	
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const TSubclassOf EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToGrant))
		{
			UE_LOG(LogHACF, Error, TEXT("GrantedGameplayEffects[%d] on ability set [%s] is not valid"), EffectIndex, *GetNameSafe(this));
			continue;
		}
		
		const UGameplayEffect* GameplayEffect = EffectToGrant->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->ApplyGameplayEffectToSelf(GameplayEffect, 1.f, ASC->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}
}
