// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "HACF_CombatComponent.generated.h"

class UAbilitySystemComponent;
struct FGameplayAbilitySpecHandle;

/*
 *
 */
UCLASS( ClassGroup=(HyperActionCombatFramework), meta=(BlueprintSpawnableComponent) )
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_CombatComponent : public UPawnComponent, IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHACF_CombatComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//~Begin IGameFrameworkInitStateInterface
	virtual FName GetFeatureName() const override;
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,FGameplayTag DesiredState) override;
	virtual void CheckDefaultInitialization() override;
	//~End
	
	void InitializeWithASC(UAbilitySystemComponent* InASC);
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	// 입력 처리
	// PlayerController의 PostProcessInput에서 호출
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	
	UAbilitySystemComponent* GetAbilitySystemComponent() const { return ASC; }

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
