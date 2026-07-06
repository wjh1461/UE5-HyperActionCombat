// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HACF_HeroComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Core/HACF_CombatComponent.h"
#include "Input/HACF_InputConfig.h"


// Sets default values for this component's properties
UHACF_HeroComponent::UHACF_HeroComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
}

// Called every frame
void UHACF_HeroComponent::TickComponent(float DeltaTime, ELevelTick TickType,
										FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}


// Called when the game starts
void UHACF_HeroComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CombatComponent = GetOwner()->FindComponentByClass<UHACF_CombatComponent>();
	
	if(APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (Pawn->InputComponent)
		{
			InitializePlayerInput(Pawn->InputComponent);
		}
	}
}

void UHACF_HeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	check(EnhancedInputComponent);

	const APawn* Pawn = Cast<APawn>(GetOwner());
	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	check(PlayerController);

	const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->GetLocalPlayer());
	check(LocalPlayer);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);
	
	Subsystem->ClearAllMappings();

	if (InputConfig)
	{
		check(InputConfig);

		// InputMappingContext 추가
		for (const FInputMappingContextAndPriority& Mapping : DefaultMappings)
		{
			if (UInputMappingContext* IMC = Mapping.InputMapping.LoadSynchronous())
			{
				FModifyContextOptions Options = {};
				Options.bIgnoreAllPressedKeysUntilRelease = false;
				
				Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);	
			}
		}

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
	//TODO: 캐릭터 이동 처리
}

void UHACF_HeroComponent::Input_Look(const FInputActionValue& InputActionValue)
{
	//TODO: 카메라 회전 처리
}

void UHACF_HeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	check(CombatComponent);
	
	CombatComponent->AbilityInputTagPressed(InputTag);
}

void UHACF_HeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	check(CombatComponent);
	
	CombatComponent->AbilityInputTagReleased(InputTag);
}
