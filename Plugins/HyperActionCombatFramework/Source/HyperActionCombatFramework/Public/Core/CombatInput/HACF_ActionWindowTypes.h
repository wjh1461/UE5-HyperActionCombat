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
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag WindowTag;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer AcceptedInputTags;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Priority;
	
	UPROPERTY(BlueprintReadOnly)
	int32 WindowId;
};

/**
 * 외부 설정을 위한 구조체
 */
USTRUCT(BlueprintType)
struct FHACF_ActionWindowDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag WindowTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer AcceptedInputTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Priority = 0;
};

struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_ActionWindowContainer
{
	uint32 Open(const FHACF_ActionWindowDefinition& Definition);

	bool Close(uint32 WindowId);

	const FHACF_ActionWindow* FindHighestPriority() const;

	void Clear();

	bool IsEmpty() const
	{
		return Windows.IsEmpty();
	}

	int32 Num() const
	{
		return Windows.Num();
	}

private:
	TArray<FHACF_ActionWindow> Windows;

	// 이 컨테이너가 관리하는 Window ID 발급 상태
	uint32 NextWindowId = 1;
};
