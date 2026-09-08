// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HACF_GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Character/HACF_HeroComponent.h"
#include "Core/HACF_CombatComponent.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "GameFramework/Character.h"
#include "MoveLibrary/PlayMoverMontageCallbackProxy.h"

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

bool UHACF_GameplayAbility::HandleComboInput(const FHACF_ActionWindowInputContext& Context)
{
	return false;
}

void UHACF_GameplayAbility::StartAttack(FName NodeId)
{
	UCharacterMoverComponent* CharacterMover = GetAvatarPawnFromActorInfo()->FindComponentByClass<UCharacterMoverComponent>();
	if (!CharacterMover)
	{
		return;
	}
	
	UAnimMontage* MontageToPlay = AttackComboDefinition ? AttackComboDefinition->GetEntryNode()->Montage : AbilityMontage;
	
	UPlayMoverMontageCallbackProxy* AttackMontageProxy =
	   UPlayMoverMontageCallbackProxy::CreateProxyObjectForPlayMoverMontage(
		   CharacterMover,
		   MontageToPlay,
		   1.0f,       // PlayRate
		   0.0f,       // StartingPosition
		   NAME_None,  // StartingSection
		   true,       // StopAllMontages
		   -1.0f);     // 기본 BlendOut 사용
	
	AttackMontageProxy->OnCompleted.AddDynamic(this, &ThisClass::OnEndAbility);
	AttackMontageProxy->OnInterrupted.AddDynamic(this, &ThisClass::OnEndAbility);
}

void UHACF_GameplayAbility::OpenComboWindow(const FGameplayTag& WindowTag)
{
	UHACF_CombatComponent* CombatComponent = GetCombatComponent();
	if (!CombatComponent)
	{
		return;
	}

	if (!AttackComboDefinition)
	{
		return;
	}
	
	FHACF_ActionWindowDefinition OutActionWindowDefinition;
	
	AttackComboDefinition->GetEntryNode()->BuildActionWindowDefinition(WindowTag, OutActionWindowDefinition);
	ComboWindowHandle = CombatComponent->OpenActionWindow(OutActionWindowDefinition, this, FHACF_ActionWindowInputHandler::CreateUObject(this, &ThisClass::HandleComboInput));
}

void UHACF_GameplayAbility::CloseComboWindow()
{
	UHACF_CombatComponent* CombatComponent = GetCombatComponent();
	if (!CombatComponent || !ComboWindowHandle.IsValid())
	{
		return;
	}
	
	CombatComponent->CloseActionWindow(ComboWindowHandle);
}

void UHACF_GameplayAbility::ResetComboState()
{
	if (ComboWindowHandle.IsValid())
	{
		ComboWindowHandle.Invalidate();
	}
}

void UHACF_GameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UHACF_GameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ResetComboState();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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
