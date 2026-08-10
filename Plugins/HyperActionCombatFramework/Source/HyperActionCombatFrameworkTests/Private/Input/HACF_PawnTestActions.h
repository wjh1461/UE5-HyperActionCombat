#pragma once

#include "CQTest.h"
#include "Components/InputTestActions.h"
#include "InputAction.h"

namespace HyperActionCombatFrameworkTests
{
	struct FHACFAbilityTestAction : public FTestAction
	{
		explicit FHACFAbilityTestAction(const UInputAction* Action)
		{
			InputActionName = Action->GetName();
			InputActionValue = FInputActionValue(true);
		}
	};

	class FHACFPawnTestActions : public FInputTestActions
	{
	public:
		explicit FHACFPawnTestActions(APawn* Pawn)
			: FInputTestActions(Pawn)
		{
		}

		void PressAbilityAction(const UInputAction* Action)
		{
			PerformAction(FHACFAbilityTestAction(Action));
		}
	};
}
