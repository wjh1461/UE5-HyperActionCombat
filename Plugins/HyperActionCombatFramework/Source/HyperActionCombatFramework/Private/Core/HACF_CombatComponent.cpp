// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/HACF_CombatComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "HACF_GameplayTags.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbilityBase.h"
#include "GameFramework/PlayerState.h"

const FName UHACF_CombatComponent::NAME_ActorFeatureName("HACF.Combat");

UHACF_CombatComponent::UHACF_CombatComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called every frame
void UHACF_CombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHACF_CombatComponent::OnRegister()
{
	Super::OnRegister();

	if (!IsTemplate())
	{
		RegisterInitStateFeature();
	}
}

// Called when the game starts
void UHACF_CombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	BindOnActorInitStateChanged(NAME_ActorFeatureName, FGameplayTag(), false);
	
	CheckDefaultInitialization();
}

void UHACF_CombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	UninitializeFromASC();
	
	Super::EndPlay(EndPlayReason);
}

bool UHACF_CombatComponent::ResolveAbilitySystem(UAbilitySystemComponent*& OutASC, AActor*& OutOwnerActor) const
{
	OutASC = nullptr;
	OutOwnerActor = nullptr;

	APlayerState* PlayerState = GetPlayerState<APlayerState>();
	if (!PlayerState)
	{
		return false;
	}
	
	const IAbilitySystemInterface* AbilitySystemProvider = Cast<IAbilitySystemInterface>(PlayerState);
	if (!AbilitySystemProvider)
	{
		return false;
	}
	
	OutASC = AbilitySystemProvider->GetAbilitySystemComponent();
	OutOwnerActor = PlayerState;
	
	return IsValid(OutASC);
}

bool UHACF_CombatComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
                                               FGameplayTag DesiredState) const
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
		UAbilitySystemComponent* ResolvedASC = nullptr;
		AActor* ResolvedOwner = nullptr;

		return Pawn && ResolveAbilitySystem(ResolvedASC, ResolvedOwner);
	}

	if (CurrentState == HACF_GameplayTags::InitState_DataAvailable &&
		DesiredState == HACF_GameplayTags::InitState_DataInitialized)
	{
		return true;
	}

	if (CurrentState == HACF_GameplayTags::InitState_DataInitialized &&
		DesiredState == HACF_GameplayTags::InitState_GameplayReady)
	{
		return ASC && ASC->GetOwnerActor() != nullptr && ASC->GetAvatarActor() == Pawn;
	}
	
	return false;
}

void UHACF_CombatComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	if (CurrentState == HACF_GameplayTags::InitState_DataAvailable &&
		DesiredState == HACF_GameplayTags::InitState_DataInitialized)
	{
		UAbilitySystemComponent* ResolvedASC = nullptr;
		AActor* ResolvedOwner = nullptr;

		const bool bResolved = ResolveAbilitySystem(ResolvedASC, ResolvedOwner);

		if (!ensureMsgf(bResolved, TEXT("CombatComponent could not resolve its PlayerState ASC.")))
		{
			return;
		}

		InitializeWithASC(ResolvedASC, ResolvedOwner);
	}
}

void UHACF_CombatComponent::CheckDefaultInitialization()
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

void UHACF_CombatComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	CheckDefaultInitialization();
}

void UHACF_CombatComponent::InitializeWithASC(UAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!ensure(Pawn && InASC && InOwnerActor))
	{
		return;
	}

	if (ASC && ASC != InASC)
	{
		UninitializeFromASC();
	}

	ASC = InASC;
	
	if (ASC->GetOwnerActor() != InOwnerActor || ASC->GetAvatarActor() != Pawn)
	{
		// PlayerState, 현재 Pawn
		ASC->InitAbilityActorInfo(InOwnerActor, Pawn);        
	}
}

void UHACF_CombatComponent::UninitializeFromASC()
{
	if (!ASC)
	{
		return;
	}
	
	if (ASC->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		
		// 프로젝트 정책에 따라 취소 대상 Ability를 필터링할 수 있다.
		ASC->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		
		InputPressedSpecHandles.Reset();
		InputHeldSpecHandles.Reset();
		InputReleasedSpecHandles.Reset();
		
		ASC->RemoveAllGameplayCues();

		// PlayerState OwnerActor는 유지하고 Avatar만 해제한다.
		if (ASC->GetOwnerActor() != nullptr)
		{
			ASC->SetAvatarActor(nullptr);
		}
		else
		{
			ASC->ClearActorInfo();
		}
	}
	
	ASC = nullptr;
}

void UHACF_CombatComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (ASC == nullptr)
	{
		return;
	}
	
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

void UHACF_CombatComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (ASC == nullptr)
	{
		return;
	}
	
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


void UHACF_CombatComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
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

