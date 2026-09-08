// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/HACF_AnimNotifyState_ComboWindow.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbility.h"
#include "Core/HACF_CombatComponent.h"

void UHACF_AnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                    float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (!IsValid(MeshComp) || !WindowTag.IsValid())
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	CombatComponent = Owner->FindComponentByClass<UHACF_CombatComponent>();

	if (!IsValid(CombatComponent))
	{
		return;
	}
	
	ActiveAbility = CombatComponent->GetActiveAttackAbility();
	if (!IsValid(ActiveAbility) || !ActiveAbility->IsActive())
	{
		return;
	}

	ActiveAbility->OpenComboWindow(WindowTag);
}

void UHACF_AnimNotifyState_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (ActiveAbility)
	{
		ActiveAbility->CloseComboWindow();
	}
	
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}

void UHACF_AnimNotifyState_ComboWindow::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyBegin(BranchingPointPayload);
	
}

void UHACF_AnimNotifyState_ComboWindow::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyEnd(BranchingPointPayload);
}
