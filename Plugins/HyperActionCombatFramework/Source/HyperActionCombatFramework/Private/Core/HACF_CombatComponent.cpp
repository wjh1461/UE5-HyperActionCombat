// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/HACF_CombatComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "HACF_GameplayTags.h"
#include "HACF_LogChannels.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbility.h"
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

void UHACF_CombatComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
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
	InputBuffer.Clear();
	ClearActionWindows();
	ComboWindowBeginHandler.Unbind();
	ComboWindowEndHandler.Unbind();
	ActiveComboWindowRouteId = 0;
	
	InputPressedBufferSequences.Reset();
	ReservedBufferedInputSequences.Reset();
	
	if (!ASC)
	{
		return;
	}
	
	if (ASC->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		
		// 프로젝트 정책에 따라 취소 대상 Ability를 필터링할 수 있다.
		ASC->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		
		PressedInputTags.Reset();
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
		const bool bIsNewPress = !PressedInputTags.Contains(InputTag);
		PressedInputTags.Add(InputTag);

		if (!bIsNewPress)
		{
			return;
		}
		
		const uint32 BufferedInputSequence = RecordBufferedInputInternal(InputTag);
		if (BufferedInputSequence != 0)
		{
			// ProcessAbilityInput 이전에 Window가 열려도 활성화 원인 입력을 가져가지 못하게 한다.
			ReservedBufferedInputSequences.Add(BufferedInputSequence);
		}
		
		for (const FGameplayAbilitySpec& AbilitySpec : ASC->GetActivatableAbilities())
		{
			// 배열에 저장
			if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				if (!InputHeldSpecHandles.Contains(AbilitySpec.Handle))
				{
					InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
					InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);

					if (BufferedInputSequence != 0)
					{
						InputPressedBufferSequences.Add(AbilitySpec.Handle, BufferedInputSequence);
					}
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
		PressedInputTags.Remove(InputTag);
		
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

	// 활성화 콜백에서 새 입력이 들어와도 이번 처리와 함께 지워지지 않도록
	// 진입 시점의 Press/Release와 원인 Sequence만 분리한다.
	TArray<FGameplayAbilitySpecHandle> PressedSpecHandles = MoveTemp(InputPressedSpecHandles);
	InputPressedSpecHandles.Reset();
	TArray<FGameplayAbilitySpecHandle> ReleasedSpecHandles = MoveTemp(InputReleasedSpecHandles);
	InputReleasedSpecHandles.Reset();
	TMap<FGameplayAbilitySpecHandle, uint32> PressedBufferSequences = MoveTemp(InputPressedBufferSequences);
	InputPressedBufferSequences.Reset();
	const TSet<uint32> ReservedSequencesToRelease = ReservedBufferedInputSequences;
	
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
	for (const FGameplayAbilitySpecHandle& SpecHandle : PressedSpecHandles)
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
		const bool bActivated = ASC->TryActivateAbility(AbilitySpecHandle);
		if (!bActivated)
		{
			continue;
		}

		if (const uint32* InputSequence = PressedBufferSequences.Find(AbilitySpecHandle))
		{
			InputBuffer.Consume(*InputSequence);
		}
	}
	
	// 활성화 판정이 끝난 입력은 이제 ActionWindow가 소비할 수 있다.
	for (const uint32 InputSequence : ReservedSequencesToRelease)
	{
		ReservedBufferedInputSequences.Remove(InputSequence);
	}
	
	// Released
	for (const FGameplayAbilitySpecHandle& SpecHandle : ReleasedSpecHandles)
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
	
	// 최초 활성화에 성공하지 않은 입력만 현재 ActionWindow가 소비할 수 있다.
	TryConsumeBufferedInput();
}

void UHACF_CombatComponent::RecordBufferedInput(const FGameplayTag& InputTag)
{
	RecordBufferedInputInternal(InputTag);
}

uint32 UHACF_CombatComponent::RecordBufferedInputInternal(const FGameplayTag& InputTag)
{    
	const UWorld* World = GetWorld();
	if (!World || !InputTag.IsValid())
	{
		return 0;
	}
	
	const double CurrentTime = World->GetTimeSeconds();
	
	FHACF_BufferedInput BufferedInput;
	BufferedInput.InputTag = InputTag;
	BufferedInput.InputTime = CurrentTime;
	BufferedInput.ExpirationTime = CurrentTime + InputBufferDuration;
	
	InputBuffer.RemoveExpired(CurrentTime);
	return InputBuffer.Push(BufferedInput);
}

FHACF_ActionWindowHandle UHACF_CombatComponent::OpenActionWindow(const FHACF_ActionWindowDefinition& Definition, const UObject* Source, FHACF_ActionWindowInputHandler InputHandler)
{
	return ActionWindows.Open(Definition, Source, MoveTemp(InputHandler));
}

bool UHACF_CombatComponent::CloseActionWindow(FHACF_ActionWindowHandle Handle)
{
	if (!Handle.IsValid())
	{
		return false;
	}
	
	if (!ActionWindows.Close(Handle))
	{
		return false;
	}

	// 높은 우선순위 Window가 닫힌 뒤
	// 아래 Window가 입력을 소비할 수 있다.
	TryConsumeBufferedInput();

	return true;
}

int32 UHACF_CombatComponent::CloseAllActionWindowsFromSource(const UObject* Source)
{
	if (!Source)
	{
		return 0;
	}

	const int32 ClosedCount = ActionWindows.CloseAllFromSource(Source);

	if (ClosedCount > 0)
	{
		TryConsumeBufferedInput();
	}

	return ClosedCount;
}

void UHACF_CombatComponent::ClearActionWindows()
{
	ActionWindows.Clear();
}

bool UHACF_CombatComponent::IsActionWindowActive(FHACF_ActionWindowHandle Handle) const
{
	return ActionWindows.Contains(Handle);
}

bool UHACF_CombatComponent::ProcessBufferedInput()
{
	return TryConsumeBufferedInput();
}

FHACF_ComboWindowRouteHandle UHACF_CombatComponent::BindComboWindowRoute(FHACF_ComboWindowBeginHandler BeginHandler, FHACF_ComboWindowEndHandler EndHandler)
{
	if (!BeginHandler.IsBound() || !EndHandler.IsBound())
	{
		return {};
	}

	++NextComboWindowRouteId;
	if (NextComboWindowRouteId == 0)
	{
		++NextComboWindowRouteId;
	}

	ComboWindowBeginHandler = MoveTemp(BeginHandler);
	ComboWindowEndHandler = MoveTemp(EndHandler);
	ActiveComboWindowRouteId = NextComboWindowRouteId;

	return FHACF_ComboWindowRouteHandle(ActiveComboWindowRouteId);
}

bool UHACF_CombatComponent::UnbindComboWindowRoute(FHACF_ComboWindowRouteHandle Handle)
{
	// 이전 실행의 늦은 종료가 새 실행의 라우트를 해제하지 않도록 한다.
	if (!Handle.IsValid() || Handle.Value != ActiveComboWindowRouteId)
	{
		return false;
	}

	ComboWindowBeginHandler.Unbind();
	ComboWindowEndHandler.Unbind();
	ActiveComboWindowRouteId = 0;
	return true;
}

void UHACF_CombatComponent::RouteComboWindowBegin(const FHACF_ComboWindowKey& NotifyKey, const FGameplayTag& WindowTag)
{
	if (!NotifyKey.IsValid() || !WindowTag.IsValid())
	{
		return;
	}

	// 실행 중 콤보 전환으로 현재 라우트가 교체될 수 있으므로 복사본을 호출한다.
	const FHACF_ComboWindowBeginHandler Handler = ComboWindowBeginHandler;
	Handler.ExecuteIfBound(NotifyKey, WindowTag);
}

void UHACF_CombatComponent::RouteComboWindowEnd(const FHACF_ComboWindowKey& NotifyKey)
{
	if (!NotifyKey.IsValid())
	{
		return;
	}

	const FHACF_ComboWindowEndHandler Handler = ComboWindowEndHandler;
	Handler.ExecuteIfBound(NotifyKey);
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

bool UHACF_CombatComponent::TryConsumeBufferedInput()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const double CurrentTime = World->GetTimeSeconds();
	InputBuffer.RemoveExpired(CurrentTime);

	const FHACF_BufferedInput* FoundInput = InputBuffer.Find();
	
	if (!FoundInput)
	{
		return false;
	}

	if (ReservedBufferedInputSequences.Contains(FoundInput->Sequence))
	{
		return false;
	}
	
	const FHACF_BufferedInput BufferedInput = *FoundInput;
	FHACF_ActionWindowDispatchTarget DispatchTarget;
	if (!ActionWindows.FindDispatchTarget(BufferedInput.InputTag, BufferedInput.InputTime, DispatchTarget))
	{
		return false;
	}
	
	const bool bAccepted = TryDispatchActionWindowInput(DispatchTarget);
	if (!bAccepted)
	{
		return false;
	}

	return InputBuffer.Consume(BufferedInput.Sequence);
}

bool UHACF_CombatComponent::TryDispatchActionWindowInput(const FHACF_ActionWindowDispatchTarget& DispatchTarget)
{
	if (bIsDispatchingBufferedInput)
	{
		return false;
	}

	TGuardValue<bool> DispatchGuard(bIsDispatchingBufferedInput, true);

	if (!DispatchTarget.Handler.IsBound())
	{
		return false;
	}
	
	return DispatchTarget.Handler.Execute(DispatchTarget.Context);
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
