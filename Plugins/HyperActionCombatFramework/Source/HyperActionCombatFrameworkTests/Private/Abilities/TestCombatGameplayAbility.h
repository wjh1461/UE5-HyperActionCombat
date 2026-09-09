#pragma once

#include "AbilitySystem/Abilities/HACF_CombatGameplayAbility.h"
#include "TestCombatGameplayAbility.generated.h"

UCLASS()
class HYPERACTIONCOMBATFRAMEWORKTESTS_API UTestCombatGameplayAbility : public UHACF_CombatGameplayAbility
{
	GENERATED_BODY()

public:
	void PlayComboNodeForTest(const FName NodeId)
	{
		PlayComboNode(NodeId);
	}
};
