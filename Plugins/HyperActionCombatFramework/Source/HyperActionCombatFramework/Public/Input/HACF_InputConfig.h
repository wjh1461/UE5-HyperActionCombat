// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "HACF_InputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * 
 */
USTRUCT(BlueprintType)
struct FHACF_InputAction
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag InputTag;
};

USTRUCT()
struct FInputMappingContextAndPriority
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UInputMappingContext> InputMapping;
	
	UPROPERTY(EditAnywhere)
	int32 Priority;
};

/**
 *
 */
UCLASS(BlueprintType)
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_InputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FHACF_InputAction> AbilityInputActions;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FHACF_InputAction> NativeInputActions;
};
