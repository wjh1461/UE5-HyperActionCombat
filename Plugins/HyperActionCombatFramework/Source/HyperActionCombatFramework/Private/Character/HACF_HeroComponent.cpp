// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HACF_HeroComponent.h"

#include "EnhancedInputComponent.h"
#include "Input/HACF_InputConfig.h"


// Sets default values for this component's properties
UHACF_HeroComponent::UHACF_HeroComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called every frame
void UHACF_HeroComponent::TickComponent(float DeltaTime, ELevelTick TickType,
										FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


// Called when the game starts
void UHACF_HeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UHACF_HeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	check(EnhancedInputComponent);

	if (InputConfig)
	{
		check(InputConfig);

		// 어빌리티 액션 바인딩
		for (const FHACF_InputAction& Action : InputConfig->AbilityInputActions)
		{
			if (Action.InputAction && Action.InputTag.IsValid())
			{
				EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Triggered, this, &ThisClass::Input_AbilityInputTagPressed, Action.InputTag);
				EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Completed, this, &ThisClass::Input_AbilityInputTagReleased, Action.InputTag);
			}
		}
		
		// 일반 액션 바인딩
		//EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
		//EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
	}
}

void UHACF_HeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	
}

void UHACF_HeroComponent::Input_Look(const FInputActionValue& InputActionValue)
{
	
}

void UHACF_HeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	AbilityInputTagPressed(InputTag);
}

void UHACF_HeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	AbilityInputTagReleased(InputTag);
}

void UHACF_HeroComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// Hold
	
	// Pressed
	
	// Released
}

