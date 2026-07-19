#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "HACF_InitStateTestPlayerState.generated.h"

class UAbilitySystemComponent;

UCLASS()
class AHACF_InitStateTestPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHACF_InitStateTestPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComponent;
	}

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
