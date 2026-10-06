// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Targeting/HACF_TargetingProcessor.h"
#include "HACF_TargetSelector_SphereOverlap.generated.h"

/** 요청 액터를 중심으로 한 구체와 충돌 컴포넌트가 겹치는 액터를 후보로 수집합니다. */
UCLASS(meta = (DisplayName = "Sphere Overlap Target Selector"))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetSelector_SphereOverlap : public UHACF_TargetSelector
{
	GENERATED_BODY()

public:
	UHACF_TargetSelector_SphereOverlap();

	virtual EHACF_TargetingProcessResult Select(FHACF_TargetingExecutionContext& Context) const override;

	/** 후보 액터를 검색할 충돌 오브젝트 타입. 기본값은 Pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting|Selection")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
};
