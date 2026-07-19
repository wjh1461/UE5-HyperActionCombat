#pragma once

#include "Character/HACF_HeroComponent.h"
#include "HACF_InitStateTestHeroComponent.generated.h"

class UHACF_PawnData;

UCLASS()
class UHACF_InitStateTestHeroComponent final : public UHACF_HeroComponent
{
	GENERATED_BODY()

public:
	void SetPawnDataForTest(const UHACF_PawnData* InPawnData)
	{
		PawnData = InPawnData;
	}
};
