// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HACF_HeroComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbilityBase.h"
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

	// 임시
	//ProcessAbilityInput(DeltaTime, false);
}


// Called when the game starts
void UHACF_HeroComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
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
	AbilityInputTagPressed(InputTag);
}

void UHACF_HeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	AbilityInputTagReleased(InputTag);
}

void UHACF_HeroComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ASC->GetActivatableAbilities())
		{
			// 배열에 저장
			if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UHACF_HeroComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ASC->GetActivatableAbilities())
		{
			// 배열에 저장
			if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
				
			}
		}
	}
}

void UHACF_HeroComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();
	
	// Hold
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UHACF_GameplayAbilityBase* HACFAbilityCDO = CastChecked<UHACF_GameplayAbilityBase>(AbilitySpec->Ability);
				if (HACFAbilityCDO && HACFAbilityCDO->GetActivationPolicy() == EHACFAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.Add(AbilitySpec->Handle);
				}
			}
		}
	}
	
	// Pressed
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;
				
				if (AbilitySpec->IsActive())
				{
					ASC->AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UHACF_GameplayAbilityBase* HACFAbilityCDO = CastChecked<UHACF_GameplayAbilityBase>(AbilitySpec->Ability);
					if (HACFAbilityCDO && HACFAbilityCDO->GetActivationPolicy() == EHACFAbilityActivationPolicy::OnInputTriggered) // 입력 타입 필터링 조건 추가해야함
					{
						AbilitiesToActivate.Add(AbilitySpec->Handle);
					}
				}
			}
		}
	}
	
	// 어빌리티 실행
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		ASC->TryActivateAbility(AbilitySpecHandle);
	}
	
	// Released
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					ASC->AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}
	
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

