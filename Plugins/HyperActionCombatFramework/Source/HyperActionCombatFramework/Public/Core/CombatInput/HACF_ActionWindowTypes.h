// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HACF_ActionWindowTypes.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FHACF_ActionWindow
{
	GENERATED_BODY()
	
	UPROPERTY()
	FGameplayTag WindowTag;
	
	UPROPERTY()
	FGameplayTagContainer AcceptedInputTags;
	
	UPROPERTY()
	int32 Priority;
	
	UPROPERTY()
	uint32 WindowId;
};
