// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/HACF_AnimNotifyState_ComboWindow.h"

#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifyEndDataContext.h"
#include "Core/HACF_CombatComponent.h"

void UHACF_AnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                    float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (!WindowTag.IsValid())
	{
		return;
	}

	if (UHACF_CombatComponent* CombatComponent = FindCombatComponent(MeshComp))
	{
		CombatComponent->RouteComboWindowBegin(FHACF_ComboWindowKey::FromQueuedNotify(EventReference.GetNotifyInstanceID()), WindowTag);
	}
}

void UHACF_AnimNotifyState_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (UHACF_CombatComponent* CombatComponent = FindCombatComponent(MeshComp))
	{
		CombatComponent->RouteComboWindowEnd(FHACF_ComboWindowKey::FromQueuedNotify(EventReference.GetNotifyInstanceID()));
	}
	
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}

void UHACF_AnimNotifyState_ComboWindow::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& Payload)
{
	// Call the base Notify directly: the base BranchingPoint method dispatches the
	// queued override with INDEX_NONE. Preserve Blueprint notification only once.
	const FAnimNotifyEventReference EventReference;
	Super::NotifyBegin(Payload.SkelMeshComponent, Payload.SequenceAsset, Payload.NotifyEvent ? Payload.NotifyEvent->GetDuration() : 0.f, EventReference);

	if (WindowTag.IsValid())
	{
		if (UHACF_CombatComponent* CombatComponent = FindCombatComponent(Payload.SkelMeshComponent))
		{
			CombatComponent->RouteComboWindowBegin(FHACF_ComboWindowKey::FromBranchingPoint(Payload), WindowTag);
		}
	}
}

void UHACF_AnimNotifyState_ComboWindow::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& Payload)
{
	if (UHACF_CombatComponent* CombatComponent = FindCombatComponent(Payload.SkelMeshComponent))
	{
		CombatComponent->RouteComboWindowEnd(FHACF_ComboWindowKey::FromBranchingPoint(Payload));
	}

	FAnimNotifyEventReference EventReference;
	if (Payload.bReachedEnd)
	{
		EventReference.AddContextData<UE::Anim::FAnimNotifyEndDataContext>(true);
	}
	
	Super::NotifyEnd(Payload.SkelMeshComponent, Payload.SequenceAsset, EventReference);
}

UHACF_CombatComponent* UHACF_AnimNotifyState_ComboWindow::FindCombatComponent(USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = IsValid(MeshComp) ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UHACF_CombatComponent>() : nullptr;
}
