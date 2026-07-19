#include "InitState/HACF_InitStateTestPlayerState.h"

#include "AbilitySystemComponent.h"

AHACF_InitStateTestPlayerState::AHACF_InitStateTestPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}
