// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "HACF_HeroComponent.generated.h"

class UAbilitySystemComponent;
class UHACF_InputConfig;
struct FGameplayAbilitySpecHandle;
struct FInputMappingContextAndPriority;

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	// 입력 처리
	// =========================================================
	// PlayerController의 PostProcessInput에서 호출
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	
	// 입력 처리용 버퍼
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	
private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FInputMappingContextAndPriority> DefaultMappings;
	
	UPROPERTY()
	TObjectPtr<UHACF_InputConfig> InputConfig;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
};
