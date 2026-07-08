// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "Input/HACF_InputConfig.h"
#include "HACF_HeroComponent.generated.h"

class UInputAction;
class UHACF_PawnData;
class UHACF_CombatComponent;
class UHACF_InputConfig;
struct FInputMappingContextAndPriority;

/**
 * 
 */
UCLASS(ClassGroup=(HyperActionCombatFramework), meta=(BlueprintSpawnableComponent))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_HeroComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHACF_HeroComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// 바인딩 처리
	void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	
	// 바인딩 메서드
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	template<class UserClass, typename FuncType>
	void BindNativeAction(UEnhancedInputComponent* EnhancedIC, const UHACF_InputConfig* InputConfig, 
		const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);
	
protected:
	UPROPERTY(EditAnywhere, Category="HACF|Pawn")
	TObjectPtr<const UHACF_PawnData> PawnData;
	
	UPROPERTY(Transient)
	TObjectPtr<UHACF_CombatComponent> CombatComponent;
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

