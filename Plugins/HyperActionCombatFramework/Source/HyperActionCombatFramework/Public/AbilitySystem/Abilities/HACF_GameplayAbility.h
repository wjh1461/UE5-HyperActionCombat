// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HACF_GameplayAbility.generated.h"

class UPlayMoverMontageCallbackProxy;
struct FHACF_ActionWindowInputContext;
class UHACF_HeroComponent;
class UHACF_CombatComponent;
class APawn;
class ACharacter;
class AController;

UENUM()
enum class EHACFAbilityActivationPolicy : uint8
{
	// 입력이 Trigger 되었을 경우
	OnInputTriggered,
	
	// 입력을 누르고 있는 동안
	WhileInputActive,
	
	// avatar가 생성되었을 경우, 바로 할당
	OnSpawn,
	
	// 특정 게임플레이 이벤트 태그를 수신했을 때 발동 (예: 피격, 패리 성공)
	OnGameplayEvent
};

/**
 * 
 */
UCLASS(Abstract)
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UHACF_GameplayAbility();
	
	EHACFAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	
	UFUNCTION(BlueprintPure, Category = "HACF|Ability")
	UHACF_CombatComponent* GetCombatComponent() const;
	UFUNCTION(BlueprintPure, Category = "HACF|Ability")
	UHACF_HeroComponent* GetHeroComponent() const;

	UFUNCTION(BlueprintPure, Category = "HACF|Ability")
	APawn* GetAvatarPawnFromActorInfo() const;
	UFUNCTION(BlueprintPure, Category = "HACF|Ability")
	ACharacter* GetAvatarCharacterFromActorInfo() const;
	UFUNCTION(BlueprintPure, Category = "HACF|Ability")
	AController* GetAvatarPawnController() const;
	
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
protected:
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HACF|AbilityActivation")
	EHACFAbilityActivationPolicy ActivationPolicy;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HACF|Animations")
	TObjectPtr<UAnimMontage> AbilityMontage;
};
