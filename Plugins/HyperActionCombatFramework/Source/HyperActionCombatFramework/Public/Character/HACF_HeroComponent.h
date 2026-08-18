// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "MoverSimulationTypes.h"
#include "AbilitySystem/HACF_AbilitySet.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "Input/HACF_InputConfig.h"
#include "HACF_HeroComponent.generated.h"

class UInputAction;
class UAbilitySystemComponent;
class UHACF_PawnData;
class UHACF_CombatComponent;
class UHACF_InputConfig;
struct FInputMappingContextAndPriority;

/**
 * 
 */
UCLASS(ClassGroup=(HyperActionCombatFramework), meta=(BlueprintSpawnableComponent))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_HeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
	, public IMoverInputProducerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHACF_HeroComponent(const FObjectInitializer& ObjectInitializer);
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//~Begin IGameFrameworkInitStateInterface
	static const FName NAME_ActorFeatureName;
	
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,FGameplayTag DesiredState) override;
	virtual void CheckDefaultInitialization() override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	//~End
	
	//~Begin IMoverInputProducerInterface
	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;
	//~End
	
	void HandlePlayerStateChanged();
	
	// 이동 입력 처리를 위한 제출용 메서드
	void SubmitMoveInput(const FVector2D& MoveInput);
	void SubmitJumpInput(bool bPressed);
	
	const UInputAction* FindNativeInputAction(const FGameplayTag& InputTag) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void OnRegister() override;
	
	// 바인딩 처리
	void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	// 해당 메서드는 사용하지 않음
	template<class UserClass, typename FuncType>
	void BindNativeAction(UEnhancedInputComponent* EnhancedIC, const UHACF_InputConfig* InputConfig, 
		const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);
	
protected:
	UPROPERTY(EditAnywhere, Category="HACF|Pawn")
	TObjectPtr<const UHACF_PawnData> PawnData;
	
	UPROPERTY(Transient)
	TObjectPtr<UHACF_CombatComponent> CombatComponent;
	
	FHACF_AbilitySet_GrantedHandles GrantedAbilitySetHandles;
	TWeakObjectPtr<UAbilitySystemComponent> GrantedAbilitySystemComponent;
	
private:
	FVector2D CachedMoveInput = FVector2D::ZeroVector;

	bool bJumpPressed = false;
	bool bJumpJustPressed = false;
};

template<class UserClass, typename FuncType>
void UHACF_HeroComponent::BindNativeAction(UEnhancedInputComponent* EnhancedIC, const UHACF_InputConfig* InputConfig, 
	const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(EnhancedIC);
	
	if (const UInputAction* Action = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		EnhancedIC->BindAction(Action, TriggerEvent, Object, Func);
	}
}
