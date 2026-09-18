// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HACF_TargetingTypes.h"
#include "HACF_TargetingConfig.generated.h"

class UHACF_TargetFilter;
class UHACF_TargetScorer;
class UHACF_TargetSelector;

/**
 * 타게팅 설정을 위한 데이터 구조.
 */
UCLASS(BlueprintType)
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_TargetingConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Targeting")
	EHACF_TargetCategory TargetCategory;
	
	// 후보 수집을 위한 Selector
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category="Targeting|Pipeline")
	TArray<TObjectPtr<UHACF_TargetSelector>> Selectors;
	
	// 후보 제거를 위한 Filter
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category="Targeting|Pipeline")
	TArray<TObjectPtr<UHACF_TargetFilter>> Filters;
	
	// 후보 우선순위를 위한 Scorer
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category="Targeting|Pipeline")
	TArray<TObjectPtr<UHACF_TargetScorer>> Scorers;
	
	// 기본 타게팅 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Targeting", meta = (ClampMin = "0.001"))
	float DefaultRange = 500.f;
	
	// 타게팅 대상 정렬
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Targeting")
	EHACF_TargetSortMode SortMode = EHACF_TargetSortMode::ScoreDescending;
	
	// 최대 타겟 가능한 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "1"))
	int32 MaxTargets = 1;

	// 타겟에 본인 제외 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	bool bExcludeSourceActor = true;
};
