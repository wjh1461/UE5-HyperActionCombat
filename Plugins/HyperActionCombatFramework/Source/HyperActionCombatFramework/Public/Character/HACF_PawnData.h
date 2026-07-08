// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HACF_PawnData.generated.h"

struct FInputMappingContextAndPriority;
class UHACF_AbilitySet;
class UHACF_InputConfig;

/**
 * 
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "HACF Pawn Data", ShortTooltip = "폰을 정의하기 위해 사용되는 데이터 에셋"))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_PawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<APawn> PawnClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HACF|Abilities")
	TArray<TObjectPtr<UHACF_AbilitySet>> AbilitySets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HACF|Input")
	TObjectPtr<UHACF_InputConfig> InputConfig;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
	
	// 카메라 모드
	
};
