// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "HACF_AnimNotifyState_ComboWindow.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "HACF Combo Window"))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_AnimNotifyState_ComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HACF|Combo")
	FGameplayTag WindowTag;
};
