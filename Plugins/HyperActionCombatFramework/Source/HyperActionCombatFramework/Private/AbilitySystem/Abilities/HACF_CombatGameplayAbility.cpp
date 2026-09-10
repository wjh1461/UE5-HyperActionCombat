// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HACF_CombatGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Core/HACF_CombatComponent.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "GameFramework/Pawn.h"
#include "MoveLibrary/PlayMoverMontageCallbackProxy.h"

void UHACF_CombatGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UnbindComboWindowRoute();
	UnbindAttackMontageCallbacks();
	
	CloseAllComboWindows();
	CurrentComboNodeId = NAME_None;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHACF_CombatGameplayAbility::PlayComboNode(FName NodeId)
{
	if (!IsActive())
	{
		return;
	}

	UCharacterMoverComponent* CharacterMover = GetAvatarPawnFromActorInfo()->FindComponentByClass<UCharacterMoverComponent>();
	if (!CharacterMover)
	{
		return;
	}

	if (!IsValid(AttackComboDefinition))
	{
		return;
	}
	
	UAnimMontage* MontageToPlay = AbilityMontage;
	FName StartingSection = NAME_None;
	float PlayRate = 1.0f;
	
	if (const FHACF_ComboNode* CurrentComboNode = AttackComboDefinition->FindNode(NodeId))
	{
		MontageToPlay = CurrentComboNode->Montage;
		StartingSection = CurrentComboNode->StartSection;
		PlayRate = CurrentComboNode->PlayRate;
	}
	
	CloseAllComboWindows();
	
	// 기존 몽타주가 중단되어도 이 어빌리티를 종료하지 않도록 합니다.
	UnbindAttackMontageCallbacks();
	
	UPlayMoverMontageCallbackProxy* AttackMontageProxy =
	   UPlayMoverMontageCallbackProxy::CreateProxyObjectForPlayMoverMontage(
		   CharacterMover,
		   MontageToPlay,
		   PlayRate,       // PlayRate
		   0.0f,       // StartingPosition
		   StartingSection,  // StartingSection
		   true,       // StopAllMontages
		   -1.0f);     // 기본 BlendOut 사용
	
	if (!IsValid(AttackMontageProxy))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	CurrentComboNodeId = NodeId;
	ActiveAttackMontageProxy = AttackMontageProxy;
	
	BindComboWindowRoute();
	AttackMontageProxy->OnCompleted.AddDynamic(this, &ThisClass::OnEndAbility);
	AttackMontageProxy->OnInterrupted.AddDynamic(this, &ThisClass::OnEndAbility);
}

void UHACF_CombatGameplayAbility::ExecuteComboTransition(const FHACF_ComboTransition* ComboTransition)
{
	PlayComboNode(ComboTransition->TargetNodeId);
}

bool UHACF_CombatGameplayAbility::HandleComboInput(const FHACF_ActionWindowInputContext& Context)
{
	if (!IsActive() || ComboWindows.FindKey(Context.WindowHandle) == nullptr)
	{
		return false;
	}

	const FHACF_ComboNode* Node = IsValid(AttackComboDefinition) ? AttackComboDefinition->FindNode(CurrentComboNodeId) : nullptr;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Node || !ASC)
	{
		return false;
	}

	const FHACF_ComboTransition* Transition = Node->FindTransition(Context.WindowTag, Context.InputTag, ASC->GetOwnedGameplayTags());
	if (!Transition)
	{
		return false;
	}

	// 기존 전이 동작 유지
	ExecuteComboTransition(Transition);
	return true;
}

void UHACF_CombatGameplayAbility::OpenComboWindow(const FHACF_ComboWindowKey& NotifyKey, const FGameplayTag& WindowTag)
{
	if (!IsActive() || !NotifyKey.IsValid() || ComboWindows.Contains(NotifyKey))
	{
		return;
	}
	
	const FHACF_ComboNode* Node = IsValid(AttackComboDefinition) ? AttackComboDefinition->FindNode(CurrentComboNodeId) : nullptr;
	UHACF_CombatComponent* CombatComponent = GetCombatComponent();
	
	FHACF_ActionWindowDefinition OutActionWindowDefinition;
	if (!CombatComponent || !Node ||
		!Node->BuildActionWindowDefinition(WindowTag, OutActionWindowDefinition))
	{
		return;
	}
	
	const FHACF_ActionWindowHandle ComboWindowHandle = CombatComponent->OpenActionWindow(OutActionWindowDefinition, this, FHACF_ActionWindowInputHandler::CreateUObject(this, &ThisClass::HandleComboInput));
	if (!ComboWindowHandle.IsValid())
	{
		return;
	}

	ComboWindows.Add(NotifyKey, ComboWindowHandle);

	// 이 호출 안에서 콤보 전환이 일어날 수 있음.
	// 따라서 맵에 핸들을 저장한 뒤 실행.
	CombatComponent->ProcessBufferedInput();
}

void UHACF_CombatGameplayAbility::CloseComboWindow(const FHACF_ComboWindowKey& NotifyKey)
{
	FHACF_ActionWindowHandle ActionWindowHandle;

	if (!ComboWindows.RemoveAndCopyValue(NotifyKey, ActionWindowHandle))
	{
		return;
	}

	if (UHACF_CombatComponent* CombatComponent = GetCombatComponent())
	{
		CombatComponent->CloseActionWindow(ActionWindowHandle);
	}
}

void UHACF_CombatGameplayAbility::CloseAllComboWindows()
{    
	ComboWindows.Reset();

	if (UHACF_CombatComponent* Combat = GetCombatComponent())
	{
		Combat->CloseAllActionWindowsFromSource(this);
	}
}

void UHACF_CombatGameplayAbility::BindComboWindowRoute()
{
	UnbindComboWindowRoute();

	UHACF_CombatComponent* CombatComponent = GetCombatComponent();
	if (!CombatComponent)
	{
		return;
	}

	ComboWindowRouteHandle = CombatComponent->BindComboWindowRoute(
		FHACF_ComboWindowBeginHandler::CreateUObject(this, &ThisClass::HandleComboWindowBegin),
		FHACF_ComboWindowEndHandler::CreateUObject(this, &ThisClass::HandleComboWindowEnd));

	if (ComboWindowRouteHandle.IsValid())
	{
		BoundComboWindowRouteComponent = CombatComponent;
	}
}

void UHACF_CombatGameplayAbility::UnbindComboWindowRoute()
{
	if (UHACF_CombatComponent* CombatComponent = BoundComboWindowRouteComponent.Get())
	{
		CombatComponent->UnbindComboWindowRoute(ComboWindowRouteHandle);
	}

	ComboWindowRouteHandle.Reset();
	BoundComboWindowRouteComponent.Reset();
}

void UHACF_CombatGameplayAbility::HandleComboWindowBegin(const FHACF_ComboWindowKey& NotifyKey, const FGameplayTag& WindowTag)
{
	if (IsActive())
	{
		OpenComboWindow(NotifyKey, WindowTag);
	}
}

void UHACF_CombatGameplayAbility::HandleComboWindowEnd(const FHACF_ComboWindowKey& NotifyKey)
{
	CloseComboWindow(NotifyKey);
}

void UHACF_CombatGameplayAbility::UnbindAttackMontageCallbacks()
{
	if (!ActiveAttackMontageProxy)
	{
		return;
	}

	// 어빌리티가 등록한 콜백만 제거합니다.
	ActiveAttackMontageProxy->OnCompleted.RemoveDynamic(this, &ThisClass::OnEndAbility);
	ActiveAttackMontageProxy->OnInterrupted.RemoveDynamic(this, &ThisClass::OnEndAbility);

	ActiveAttackMontageProxy = nullptr;
}
