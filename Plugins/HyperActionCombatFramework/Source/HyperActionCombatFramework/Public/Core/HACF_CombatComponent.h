// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "HACF_CombatComponent.generated.h"

class UAbilitySystemComponent;
struct FGameplayAbilitySpecHandle;

/*
 *
 */
UCLASS( ClassGroup=(HyperActionCombatFramework), meta=(BlueprintSpawnableComponent) )
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_CombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHACF_CombatComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void InitializeWithASC(UAbilitySystemComponent* InASC);
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	// 입력 처리
	// PlayerController의 PostProcessInput에서 호출
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// 입력 처리용 버퍼
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	
private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
};
