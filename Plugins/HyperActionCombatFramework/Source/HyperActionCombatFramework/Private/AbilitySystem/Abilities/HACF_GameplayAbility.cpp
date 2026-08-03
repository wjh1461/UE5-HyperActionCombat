// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HACF_GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Character/HACF_HeroComponent.h"
#include "Core/HACF_CombatComponent.h"
#include "GameFramework/Character.h"

UHACF_GameplayAbility::UHACF_GameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

UHACF_CombatComponent* UHACF_GameplayAbility::GetCombatComponent() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	return AvatarActor ? AvatarActor->FindComponentByClass<UHACF_CombatComponent>() : nullptr;
}

UHACF_HeroComponent* UHACF_GameplayAbility::GetHeroComponent() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	return AvatarActor ? AvatarActor->FindComponentByClass<UHACF_HeroComponent>() : nullptr;
}

APawn* UHACF_GameplayAbility::GetAvatarPawnFromActorInfo() const
{
	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	return AvatarPawn;
}

ACharacter* UHACF_GameplayAbility::GetAvatarCharacterFromActorInfo() const
{
	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	return AvatarCharacter;
}

AController* UHACF_GameplayAbility::GetAvatarPawnController() const
{
	APawn* AvatarPawn = GetAvatarPawnFromActorInfo();
	return AvatarPawn ? AvatarPawn->GetController() : nullptr;
}

void UHACF_GameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	
	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UHACF_GameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	if (ActivationPolicy != EHACFAbilityActivationPolicy::OnSpawn)
	{
		return;
	}

	if (!ActorInfo || Spec.IsActive())
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();;
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();

	if (!ASC || !AvatarActor)
	{
		return;
	}

	ASC->TryActivateAbility(Spec.Handle);
}
