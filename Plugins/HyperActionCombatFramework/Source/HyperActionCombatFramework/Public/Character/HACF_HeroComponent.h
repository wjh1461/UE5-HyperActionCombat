// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "HACF_HeroComponent.generated.h"

class UHACF_CombatComponent;
class UHACF_InputConfig;
struct FInputMappingContextAndPriority;

/**
 * 
 */
UCLASS(ClassGroup=(HACF), meta=(BlueprintSpawnableComponent))
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
	// =========================================================
	void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	
	// 바인딩 메서드
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FInputMappingContextAndPriority> DefaultMappings;
	
	UPROPERTY()
	TObjectPtr<UHACF_InputConfig> InputConfig;
	
	UPROPERTY()
	TObjectPtr<UHACF_CombatComponent> CombatComponent;
};
