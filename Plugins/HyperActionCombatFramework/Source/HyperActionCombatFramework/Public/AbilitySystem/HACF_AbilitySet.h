// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "HACF_AbilitySet.generated.h"

struct FActiveGameplayEffectHandle;
struct FGameplayAbilitySpecHandle;
class UGameplayEffect;
class UAbilitySystemComponent;
class UHACF_GameplayAbility;

USTRUCT(BlueprintType)
struct FHACF_AbilitySet_GameplayAbility
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UHACF_GameplayAbility> Ability;
	
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;
	
	UPROPERTY(EditDefaultsOnly, meta=(Categories = "Input"))
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FHACF_AbilitySet_GrantedHandles
{
	GENERATED_BODY()
	
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	
	void TakeFromAbilitySystem(UAbilitySystemComponent* ASC);
	
protected:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
	
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;
};


/**
 * Gameplay Abilities 부여하기 위한 데이터 에셋
 */
UCLASS()
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_AbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC, FHACF_AbilitySet_GrantedHandles* OutGrantedHandles, 
		UObject* SourceObject = nullptr) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FHACF_AbilitySet_GameplayAbility> GrantedGameplayAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> GrantedGameplayEffects;
};
