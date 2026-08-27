// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CombatInput/HACF_InputBuffer.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "HACF_CombatComponent.generated.h"

struct FGameplayAbilitySpec;
class UAbilitySystemComponent;
struct FGameplayAbilitySpecHandle;

/*
 *
 */
UCLASS( ClassGroup=(HyperActionCombatFramework), meta=(BlueprintSpawnableComponent) )
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_CombatComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHACF_CombatComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//~Begin IGameFrameworkInitStateInterface
	static const FName NAME_ActorFeatureName;
	
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,FGameplayTag DesiredState) override;
	virtual void CheckDefaultInitialization() override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	//~End
	
	void HandlePlayerStateChanged();
	
	void InitializeWithASC(UAbilitySystemComponent* InASC, AActor* InOwnerActor);
	void UninitializeFromASC();
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	// 입력 처리
	// PlayerController의 PostProcessInput에서 호출
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	
	void RecordBufferedInput(const FGameplayTag& InputTag);
	
	UFUNCTION(BlueprintCallable, Category="HACF")
	UAbilitySystemComponent* GetAbilitySystemComponent() const { return ASC; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void OnRegister() override;
	
	virtual bool ResolveAbilitySystem(UAbilitySystemComponent*& OutASC, AActor*& OutOwnerActor) const;
	
	// ASC AbilityInput 이벤트 전달
	void DispatchAbilityInputPressed(FGameplayAbilitySpec& AbilitySpec);
	void DispatchAbilityInputReleased(FGameplayAbilitySpec& AbilitySpec);
	
	// 입력 처리용 버퍼
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	
	FHACF_InputBuffer InputBuffer;
	
	UPROPERTY(EditDefaultsOnly)
	float InputBufferDuration = 0.2f;
	
private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
};
