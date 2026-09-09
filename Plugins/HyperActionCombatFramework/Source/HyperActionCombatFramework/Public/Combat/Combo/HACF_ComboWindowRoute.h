#pragma once

#include "CoreMinimal.h"
#include "Combat/Combo/HACF_ComboWindowKey.h"
#include "GameplayTagContainer.h"

class UHACF_CombatComponent;

struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_ComboWindowRouteHandle
{
	FHACF_ComboWindowRouteHandle() = default;

	bool IsValid() const
	{
		return Value != 0;
	}

	void Reset()
	{
		Value = 0;
	}

	bool operator==(const FHACF_ComboWindowRouteHandle& Other) const
	{
		return Value == Other.Value;
	}

private:
	explicit FHACF_ComboWindowRouteHandle(uint32 InValue)
		: Value(InValue)
	{
	}

	uint32 Value = 0;

	friend UHACF_CombatComponent;
};

DECLARE_DELEGATE_TwoParams(FHACF_ComboWindowBeginHandler, const FHACF_ComboWindowKey&, const FGameplayTag&);
DECLARE_DELEGATE_OneParam(FHACF_ComboWindowEndHandler, const FHACF_ComboWindowKey&);
