// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HACF_HeroComponent.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HACF_GameplayTags.h"
#include "HACF_LogChannels.h"
#include "InputMappingContext.h"
#include "AbilitySystem/HACF_AbilitySet.h"
#include "Character/HACF_PawnData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Core/HACF_CombatComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Input/HACF_InputConfig.h"

const FName UHACF_HeroComponent::NAME_ActorFeatureName("HACF.Hero");

UHACF_HeroComponent::UHACF_HeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

// Called every frame
void UHACF_HeroComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHACF_HeroComponent::OnRegister()
{
	Super::OnRegister();
	
	if (!IsTemplate())
	{
		ensureMsgf(GetPawn<APawn>() != nullptr, TEXT("HACF_HeroComponent must be attached to a Pawn."));

		RegisterInitStateFeature();
	}
}

// Called when the game starts
void UHACF_HeroComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!PawnData)
	{
		UE_LOG(LogHACFInitState, Error, TEXT("[%s] HeroComponent cannot initialize: PawnData is not configured. Owner=%s"), *GetNameSafe(this), *GetNameSafe(GetOwner()));
	}

	const APawn* Pawn = GetPawn<APawn>();
	if (Pawn && !Pawn->FindComponentByClass<UHACF_CombatComponent>())
	{
		UE_LOG(LogHACFInitState, Error, TEXT("[%s] HeroComponent cannot initialize: required CombatComponent is missing. Pawn=%s"), *GetNameSafe(this), *GetNameSafe(Pawn));
	}
	
	BindOnActorInitStateChanged(UHACF_CombatComponent::NAME_ActorFeatureName, FGameplayTag(), true);
	
	ensure(TryToChangeInitState(HACF_GameplayTags::InitState_Spawned));
	
	CheckDefaultInitialization();
}

void UHACF_HeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	CombatComponent = nullptr;
	
	Super::EndPlay(EndPlayReason);
}

bool UHACF_HeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	const APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() &&
		DesiredState == HACF_GameplayTags::InitState_Spawned)
	{
		return Pawn != nullptr;
	}

	if (CurrentState == HACF_GameplayTags::InitState_Spawned &&
		DesiredState == HACF_GameplayTags::InitState_DataAvailable)
	{
		if (!Pawn || !PawnData)
		{
			return false;
		}

		if (!Pawn->FindComponentByClass<UHACF_CombatComponent>())
		{
			return false;
		}

		// Simulated Proxy와 원격 서버 Pawn은 입력 컴포넌트를 기다리지 않는다.
		if (Pawn->IsLocallyControlled() && !Pawn->IsBotControlled())
		{
			const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
			if (!PlayerController || !PlayerController->GetLocalPlayer() || !Pawn->InputComponent)
			{
				return false;
			}
		}
		
		return true;
	}

	if (CurrentState == HACF_GameplayTags::InitState_DataAvailable &&
		DesiredState == HACF_GameplayTags::InitState_DataInitialized)
	{
		return Manager->HasFeatureReachedInitState(GetOwner(), UHACF_CombatComponent::NAME_ActorFeatureName,
			HACF_GameplayTags::InitState_DataInitialized);
	}

	if (CurrentState == HACF_GameplayTags::InitState_DataInitialized &&
		DesiredState == HACF_GameplayTags::InitState_GameplayReady)
	{
		return Manager->HasFeatureReachedInitState(GetOwner(), UHACF_CombatComponent::NAME_ActorFeatureName, 
			HACF_GameplayTags::InitState_GameplayReady);
	}

	return false;
}

void UHACF_HeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	if (CurrentState == HACF_GameplayTags::InitState_DataAvailable && 
		DesiredState == HACF_GameplayTags::InitState_DataInitialized)
	{
		CombatComponent = GetOwner()->FindComponentByClass<UHACF_CombatComponent>();
		
		if (!CombatComponent)
		{
			UE_LOG(LogHACFInitState, Error, TEXT("[%s] DataInitialized transition aborted: CombatComponent is missing. Owner=%s"), *GetNameSafe(this), *GetNameSafe(GetOwner()));
			return;
		}
		
		UAbilitySystemComponent* ASC = CombatComponent->GetAbilitySystemComponent();
		
		if (!ASC)
		{
			UE_LOG(LogHACFInitState, Error, TEXT("[%s] DataInitialized transition aborted: CombatComponent has no initialized ASC. Owner=%s, CombatComponent=%s"), 
				*GetNameSafe(this), *GetNameSafe(GetOwner()), *GetNameSafe(CombatComponent));
			return;
		}
		
		// AbilitySet 부여는 서버만 수행한다.
		if (ASC->IsOwnerActorAuthoritative())
		{
			for (const UHACF_AbilitySet* AbilitySet : PawnData->AbilitySets)
			{
				if (AbilitySet)
				{
					AbilitySet->GiveToAbilitySystem(ASC, &GrantedAbilitySetHandles, GetOwner());
				}
			}
		}
		
		if (APawn* Pawn = GetPawn<APawn>())
		{
			if (Pawn->IsLocallyControlled() && !Pawn->IsBotControlled())
			{
				check(Pawn->InputComponent);
				InitializePlayerInput(Pawn->InputComponent);
			}
		}
	}
}

void UHACF_HeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain =
	{
		HACF_GameplayTags::InitState_Spawned,
		HACF_GameplayTags::InitState_DataAvailable,
		HACF_GameplayTags::InitState_DataInitialized,
		HACF_GameplayTags::InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}

void UHACF_HeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UHACF_CombatComponent::NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void UHACF_HeroComponent::HandlePlayerStateChanged()
{
	CheckDefaultInitialization();
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
	
	if (!PawnData)
	{
		UE_LOG(LogHACFInitState, Error, TEXT("[%s] Input initialization failed: PawnData is missing."), *GetNameSafe(this));
		return;
	}

	const UHACF_InputConfig* InputConfig = PawnData->InputConfig;
	if (!InputConfig)
	{
		UE_LOG(LogHACFInitState, Error, TEXT("[%s] Input initialization failed: PawnData %s has no InputConfig."), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	if (PawnData)
	{
		if (InputConfig)
		{
			// InputMappingContext 추가
			for (const FInputMappingContextAndPriority& Mapping : PawnData->DefaultInputMappings)
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
					EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Canceled, this, &ThisClass::Input_AbilityInputTagReleased, Action.InputTag);
				}
			}
			
			// 일반 액션 바인딩
			// BindNativeAction(EnhancedInputComponent, InputConfig, FGameplayTag(), ETriggerEvent::Triggered,this, &ThisClass::Input_Move, false);
			// BindNativeAction(EnhancedInputComponent, InputConfig, FGameplayTag(), ETriggerEvent::Triggered,this, &ThisClass::Input_Look, false);
		}
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


