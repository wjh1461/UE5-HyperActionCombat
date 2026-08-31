// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HACF_InputBufferTypes.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FHACF_BufferedInput
{
	GENERATED_BODY()

	uint32 Sequence = 0;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly)
	float InputTime = 0.0;

	UPROPERTY(BlueprintReadOnly)
	float ExpirationTime = 0.0;

	bool IsExpired(const float CurrentTime) const
	{
		return CurrentTime >= ExpirationTime;
	}
};
