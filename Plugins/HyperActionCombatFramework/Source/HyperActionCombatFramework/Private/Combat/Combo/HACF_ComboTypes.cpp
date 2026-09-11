// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Combo/HACF_ComboTypes.h"

#include "Core/CombatInput/HACF_ActionWindowTypes.h"

const FHACF_ComboTransition* FHACF_ComboNode::FindTransition(const FGameplayTag& WindowTag, const FGameplayTag& InputTag, const FGameplayTagContainer& OwnedTags) const
{
	if (!WindowTag.IsValid() || !InputTag.IsValid())
	{
		return nullptr;
	}

	const FHACF_ComboTransition* BestTransition = nullptr;

	for (const FHACF_ComboTransition& Transition : Transitions)
	{
		if (!Transition.Matches(WindowTag, InputTag, OwnedTags))
		{
			continue;
		}

		// 우선순위가 같다면 배열에서 먼저 정의된 전이를 유지합니다.
		if (!BestTransition || Transition.Priority > BestTransition->Priority)
		{
			BestTransition = &Transition;
		}
	}

	return BestTransition;
}

bool FHACF_ComboNode::BuildActionWindowDefinition(const FGameplayTag& WindowTag, FHACF_ActionWindowDefinition& OutDefinition) const
{
	OutDefinition = FHACF_ActionWindowDefinition{};

	if (!WindowTag.IsValid())
	{
		return false;
	}

	const FHACF_ComboWindow* Window = Windows.FindByPredicate(
		[&WindowTag](const FHACF_ComboWindow& Candidate)
		{
			return Candidate.WindowTag == WindowTag;
		});

	if (!Window)
	{
		return false;
	}

	FHACF_ActionWindowDefinition Definition;
	Definition.WindowTag = Window->WindowTag;
	Definition.Priority = Window->Priority;

	// 허용 입력은 전이 목록에서 생성하여 중복 설정을 피합니다.
	for (const FHACF_ComboTransition& Transition : Transitions)
	{
		if (Transition.WindowTag == WindowTag && Transition.InputTag.IsValid())
		{
			Definition.AcceptedInputTags.AddTag(Transition.InputTag);
		}
	}

	if (Definition.AcceptedInputTags.IsEmpty())
	{
		return false;
	}

	OutDefinition = MoveTemp(Definition);
	return true;
}
