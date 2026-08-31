// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/HACF_CombatComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "HACF_GameplayTags.h"
#include "HACF_LogChannels.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbility.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/PlayerState.h"

const FName UHACF_CombatComponent::NAME_ActorFeatureName("HACF.Combat");

UHACF_CombatComponent::UHACF_CombatComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
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
	
	//BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
	
	ensure(TryToChangeInitState(HACF_GameplayTags::InitState_Spawned));
	
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

	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return false;
	}
	
	// PlayerState에 AbilitySystemComponent가 있는 경우. 플레이어용
	if (APlayerState* PlayerState = GetPlayerState<APlayerState>())
	{
		if (const IAbilitySystemInterface* Provider = Cast<IAbilitySystemInterface>(PlayerState))
		{
			OutASC = Provider->GetAbilitySystemComponent();
			OutOwnerActor = PlayerState;
			
			return IsValid(OutASC);
		}
	}
	
	// Pawn에 AbilitySystemComponent가 있는 경우(PlayerState 대신). NPC, 적
	if (const IAbilitySystemInterface* AbilitySystemProvider = Cast<IAbilitySystemInterface>(Pawn))
	{
		if (UAbilitySystemComponent* PawnASC = AbilitySystemProvider->GetAbilitySystemComponent())
		{
			OutASC = PawnASC;
			OutOwnerActor = Pawn;
			
			return IsValid(OutASC);
		}
	}
	
	return false;
}

void UHACF_CombatComponent::DispatchAbilityInputPressed(FGameplayAbilitySpec& AbilitySpec)
{
	ASC->AbilitySpecInputPressed(AbilitySpec);
	
	if (!Cast<UHACF_GameplayAbility>(AbilitySpec.Ability))
	{
		return;
	}
	
	const UGameplayAbility* Instance = AbilitySpec.GetPrimaryInstance();
	if (!ensure(Instance))
	{
		return;
	}
	
	const FPredictionKey OriginalPredictionKey = Instance->GetCurrentActivationInfo().GetActivationPredictionKey();

	ASC->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, OriginalPredictionKey);
}

void UHACF_CombatComponent::DispatchAbilityInputReleased(FGameplayAbilitySpec& AbilitySpec)
{
	ASC->AbilitySpecInputReleased(AbilitySpec);
	
	if (!Cast<UHACF_GameplayAbility>(AbilitySpec.Ability))
	{
		return;
	}
	
	const UGameplayAbility* Instance = AbilitySpec.GetPrimaryInstance();
	if (!ensure(Instance))
	{
		return;
	}
	
	const FPredictionKey OriginalPredictionKey = Instance->GetCurrentActivationInfo().GetActivationPredictionKey();

	ASC->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, OriginalPredictionKey);
}

bool UHACF_CombatComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	APawn* Pawn = GetPawn<APawn>();
	
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
		if (!bResolved)
		{
			UE_LOG(LogHACFInitState, Error, TEXT("[%s] DataInitialized transition aborted: failed to resolve PlayerState ASC. Pawn=%s, PlayerState=%s"),
				*GetNameSafe(this),
				*GetNameSafe(GetPawn<APawn>()),
				*GetNameSafe(GetPlayerState<APlayerState>()));
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

void UHACF_CombatComponent::HandlePlayerStateChanged()
{
	APlayerState* PlayerState = GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		const IAbilitySystemInterface* AbilitySystemProvider = Cast<IAbilitySystemInterface>(PlayerState);
		if (!AbilitySystemProvider)
		{
			UE_LOG(LogHACFInitState, Error, TEXT("[%s] CombatComponent cannot initialize: PlayerState %s does not implement IAbilitySystemInterface. Pawn=%s"),
				*GetNameSafe(this),
				*GetNameSafe(PlayerState),
				*GetNameSafe(GetPawn<APawn>()));
		}
		else if (!AbilitySystemProvider->GetAbilitySystemComponent())
		{
			UE_LOG(LogHACFInitState, Error, TEXT("[%s] CombatComponent cannot initialize: PlayerState %s returned a null ASC. Pawn=%s"),
				*GetNameSafe(this),
				*GetNameSafe(PlayerState),
				*GetNameSafe(GetPawn<APawn>()));
		}
	}
	
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
	
	InputBuffer.Clear();
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
		RecordBufferedInput(InputTag);
		
		for (const FGameplayAbilitySpec& AbilitySpec : ASC->GetActivatableAbilities())
		{
			// 배열에 저장
			if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				if (!InputHeldSpecHandles.Contains(AbilitySpec.Handle))
				{
					InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
					InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
				}
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
				if (InputHeldSpecHandles.Remove(AbilitySpec.Handle) > 0)
				{
					InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				}
			}
		}
	}
}


void UHACF_CombatComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (!IsValid(ASC))
	{
		return;
	}
	
	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();
	
	// Hold
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UHACF_GameplayAbility* HACFAbilityCDO = CastChecked<UHACF_GameplayAbility>(AbilitySpec->Ability);
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
					DispatchAbilityInputPressed(*AbilitySpec);
				}
				else
				{
					const UHACF_GameplayAbility* HACFAbilityCDO = CastChecked<UHACF_GameplayAbility>(AbilitySpec->Ability);
					if (HACFAbilityCDO && HACFAbilityCDO->GetActivationPolicy() == EHACFAbilityActivationPolicy::OnInputTriggered)
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
					DispatchAbilityInputReleased(*AbilitySpec);
				}
			}
		}
	}
	
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UHACF_CombatComponent::RecordBufferedInput(const FGameplayTag& InputTag)
{
	const double CurrentTime = GetWorld()->GetTimeSeconds();
	
	FHACF_BufferedInput BufferedInput;
	BufferedInput.InputTag = InputTag;
	BufferedInput.InputTime = CurrentTime;
	BufferedInput.ExpirationTime = CurrentTime + InputBufferDuration;
	
	InputBuffer.RemoveExpired(CurrentTime);
	InputBuffer.Push(BufferedInput);
}
