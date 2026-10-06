// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Targeting/HACF_TargetingProcessor.h"
#include "HACF_TargetScorer_AimAndDistance.generated.h"

/**
 * 액터가 향하는 방향과 거리를 기준으로 가까울수록 높은 점수.
 */
UCLASS(meta=(DisplayName = "Aim And Distance Target Scorer"))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetScorer_AimAndDistance : public UHACF_TargetScorer
{
	GENERATED_BODY()
	
public:
	virtual EHACF_TargetingProcessResult Score(FHACF_TargetingExecutionContext& Context) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting|Scoring", meta = (ClampMin = "0.0"))
	float AimWeight = 0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting|Scoring", meta = (ClampMin = "0.0"))
	float DistanceWeight = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting|Scoring", meta = (ClampMin = "0.0"))
	float PreferredTargetBonus = 0.1f;
};
