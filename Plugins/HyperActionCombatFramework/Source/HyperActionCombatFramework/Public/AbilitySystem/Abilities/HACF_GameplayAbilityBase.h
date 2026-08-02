// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HACF_GameplayAbilityBase.generated.h"

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
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_GameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UHACF_GameplayAbilityBase();
	
	EHACFAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HACF|AbilityActivation")
	EHACFAbilityActivationPolicy ActivationPolicy;
};
