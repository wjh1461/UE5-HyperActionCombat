#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CQTest.h"
#include "Abilities/TestGameplayAbility.h"
#include "Character/HACF_HeroComponent.h"
#include "Character/HACF_PawnData.h"
#include "Components/InputTestActions.h"
#include "Components/MapTestSpawner.h"
#include "Core/HACF_CombatComponent.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Input/HACF_InputConfig.h"

namespace HyperActionCombatFrameworkTests
{
	struct FHACFAbilityTestAction : public FTestAction
	{
		explicit FHACFAbilityTestAction(const UInputAction* Action)
		{
			InputActionName = Action->GetName();
			InputActionValue = FInputActionValue(true);
		}
	};

	class FHACFPawnTestActions : public FInputTestActions
	{
	public:
		explicit FHACFPawnTestActions(APawn* Pawn)
			: FInputTestActions(Pawn)
		{
		}

		void PressAbilityAction(const UInputAction* Action)
		{
			PerformAction(FHACFAbilityTestAction(Action));
		}
	};

	class UHACF_HeroComponent_Accessor : public UHACF_HeroComponent
	{
	public:
		using UHACF_HeroComponent::InitializePlayerInput;
		using UHACF_HeroComponent::CombatComponent;
		using UHACF_HeroComponent::PawnData;
	};

	TEST_CLASS(HACF_InputSimulationTest, "HyperActionCombat.Tests.InputSimulation")
	{
		class UHACF_CombatComponent_Accessor : public UHACF_CombatComponent
		{
		public:
			using UHACF_CombatComponent::InputPressedSpecHandles;
			using UHACF_CombatComponent::InputHeldSpecHandles;
			using UHACF_CombatComponent::InputReleasedSpecHandles;
		};

		TUniquePtr<FMapTestSpawner> Spawner;
		UHACF_InputConfig* InputConfig = nullptr;
		
		APawn* TestPawn = nullptr;
		UAbilitySystemComponent* AbilitySystemComponent = nullptr;
		UHACF_CombatComponent_Accessor* CombatComponent = nullptr;
		UHACF_HeroComponent_Accessor* HeroComponent = nullptr;
		
		UHACF_PawnData* PawnData = nullptr;
		
		TUniquePtr<FHACFPawnTestActions> PawnActions;
		
		FGameplayAbilitySpecHandle GivenHandle;

		BEFORE_EACH()
		{
			Spawner = FMapTestSpawner::CreateFromTempLevel(TestCommandBuilder);
			Spawner->AddWaitUntilLoadedCommand(TestRunner);

			PawnData = Cast<UHACF_PawnData>(StaticLoadObject(UHACF_PawnData::StaticClass(), nullptr,
				TEXT("/Script/HyperActionCombatFramework.HACF_PawnData'/Game/Data/CombatPawnData.CombatPawnData'")));
			InputConfig = PawnData->InputConfig;
		}

		AFTER_EACH()
		{
			if (TestPawn)
			{
				TestPawn->ConditionalBeginDestroy();
			}
		}

		void SetupTestEnvironment()
		{
			TestCommandBuilder
				.StartWhen([this]() { return Spawner->FindFirstPlayerPawn() != nullptr; })
				.Then([this]() {
					TestPawn = Spawner->FindFirstPlayerPawn();
					PawnActions = MakeUnique<FHACFPawnTestActions>(TestPawn);

					AbilitySystemComponent = NewObject<UAbilitySystemComponent>(TestPawn);
					AbilitySystemComponent->RegisterComponent();

					CombatComponent = NewObject<UHACF_CombatComponent_Accessor>(TestPawn);
					CombatComponent->RegisterComponent();
					CombatComponent->InitializeWithASC(AbilitySystemComponent, TestPawn);

					HeroComponent = NewObject<UHACF_HeroComponent_Accessor>(TestPawn);
					HeroComponent->CombatComponent = CombatComponent;
					HeroComponent->PawnData = PawnData;
					HeroComponent->RegisterComponent();
					HeroComponent->InitializePlayerInput(TestPawn->GetController()->InputComponent);
				});
		}

		void VerifyInputTriggers(const FName TagName)
		{
			TestCommandBuilder
				.Do([this, TagName]() {
					const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName);
					const UInputAction* Action = InputConfig->FindAbilityInputActionForTag(Tag);
					ASSERT_THAT(IsNotNull(Action));

					FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
					Spec.GetDynamicSpecSourceTags().AddTag(Tag);
					GivenHandle = AbilitySystemComponent->GiveAbility(Spec);
					PawnActions->PressAbilityAction(Action);
				})
				.Then([this]() {
					ASSERT_THAT(IsTrue(CombatComponent->InputPressedSpecHandles.Contains(GivenHandle)));
					ASSERT_THAT(IsTrue(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));
				});
		}

		void VerifyInputReleased(const FName TagName)
		{
			TestCommandBuilder
				.Do([this, TagName]() {
					const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName);
					const UInputAction* Action = InputConfig->FindAbilityInputActionForTag(Tag);

					FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
					Spec.GetDynamicSpecSourceTags().AddTag(Tag);
					GivenHandle = AbilitySystemComponent->GiveAbility(Spec);
					PawnActions->PressAbilityAction(Action);
				})
				.Then([this]() { PawnActions->StopAllActions(); })
				.Then([this]() {
					ASSERT_THAT(IsFalse(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));
					ASSERT_THAT(IsTrue(CombatComponent->InputReleasedSpecHandles.Contains(GivenHandle)));
				});
		}

		TEST_METHOD(SimulateInput_PrimaryAttack_TriggersAbilityInputTag)
		{
			SetupTestEnvironment();
			VerifyInputTriggers(FName("HACF.Input.Action.Primary"));
		}

		TEST_METHOD(SimulateInput_PrimaryAttackReleased_ClearsAbilityInputTag)
		{
			SetupTestEnvironment();
			VerifyInputReleased(FName("HACF.Input.Action.Primary"));
		}

		TEST_METHOD(SimulateInput_SecondaryDodge_TriggersAbilityInputTag)
		{
			SetupTestEnvironment();
			VerifyInputTriggers(FName("HACF.Input.Action.Ability.Dodge"));
		}

		TEST_METHOD(SimulateInput_SecondaryReleased_ClearsAbilityInputTag)
		{
			SetupTestEnvironment();
			VerifyInputReleased(FName("HACF.Input.Action.Ability.Dodge"));
		}

		TEST_METHOD(SimulateInput_AbilityNormal_TriggersAbilityInputTag)
		{
			SetupTestEnvironment();
			VerifyInputTriggers(FName("HACF.Input.Action.Ability.Normal"));
		}

		TEST_METHOD(SimulateInput_AbilityNormalReleased_ClearsAbilityInputTag)
		{
			SetupTestEnvironment();
			VerifyInputReleased(FName("HACF.Input.Action.Ability.Normal"));
		}

		TEST_METHOD(SimulateInput_AbilityUltimate_TriggersAbilityInputTag)
		{
			SetupTestEnvironment();
			VerifyInputTriggers(FName("HACF.Input.Action.Ability.Ultimate"));
		}

		TEST_METHOD(SimulateInput_AbilityUltimateReleased_ClearsAbilityInputTag)
		{
			SetupTestEnvironment();
			VerifyInputReleased(FName("HACF.Input.Action.Ability.Ultimate"));
		}
	};

	TEST_CLASS(HACF_RuntimeInputPathTest, "HyperActionCombat.Tests.InputSimulation.RuntimePath")
	{
		TUniquePtr<FMapTestSpawner> Spawner;

		APawn* TestPawn = nullptr;
		APlayerController* PlayerController = nullptr;
		UAbilitySystemComponent* AbilitySystemComponent = nullptr;
		UHACF_CombatComponent* CombatComponent = nullptr;
		UHACF_HeroComponent* HeroComponent = nullptr;
		
		UHACF_InputConfig* InputConfig = nullptr;
		const UInputAction* PrimaryInputAction = nullptr;
		FKey PrimaryInputKey;

		FGameplayAbilitySpecHandle GivenHandle;

		BEFORE_EACH()
		{
			// 사용하는 맵은 Sample 프로젝트에 있음
			Spawner = MakeUnique<FMapTestSpawner>(TEXT("/Game/Maps"), TEXT("L_CombatTest"));
			Spawner->AddWaitUntilLoadedCommand(TestRunner);

			UHACF_PawnData* PawnData = Cast<UHACF_PawnData>(StaticLoadObject(UHACF_PawnData::StaticClass(), nullptr,
				TEXT("/Script/HyperActionCombatFramework.HACF_PawnData'/Game/Data/CombatPawnData.CombatPawnData'")));
			ASSERT_THAT(IsNotNull(PawnData));

			InputConfig = PawnData->InputConfig;
			ASSERT_THAT(IsNotNull(InputConfig));
		}

		AFTER_EACH()
		{
			if (PlayerController && PrimaryInputKey.IsValid())
			{
				PlayerController->InputKey(FInputKeyEventArgs::CreateSimulated(PrimaryInputKey, IE_Released, 0.0f));
			}
		}

		TEST_METHOD(PrimaryInput_ActivatesAbilityThroughRuntimePath)
		{
			TestCommandBuilder
				.StartWhen([this]() { return Spawner->FindFirstPlayerPawn() != nullptr; })
				.Then([this]() {
					// 테스트용 맵에 스폰되어 있는 폰
					TestPawn = Spawner->FindFirstPlayerPawn();
					ASSERT_THAT(IsNotNull(TestPawn));

					// Sample 프로젝트에서 현재 사용 중인 SamplePlayerController
					PlayerController = Cast<APlayerController>(TestPawn->GetController());
					ASSERT_THAT(IsNotNull(PlayerController));

					UClass* SamplePlayerControllerClass = StaticLoadClass(APlayerController::StaticClass(), nullptr,
						TEXT("/Script/HACF_Sample.SamplePlayerController"));
					ASSERT_THAT(IsNotNull(SamplePlayerControllerClass));
					ASSERT_THAT(IsTrue(PlayerController->IsA(SamplePlayerControllerClass)));

					CombatComponent = TestPawn->FindComponentByClass<UHACF_CombatComponent>();
					HeroComponent = TestPawn->FindComponentByClass<UHACF_HeroComponent>();
					ASSERT_THAT(IsNotNull(CombatComponent));
					ASSERT_THAT(IsNotNull(HeroComponent));

					// PlayerState에 있는 AbilitySystemComponent 찾기
					APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>();
					ASSERT_THAT(IsNotNull(PlayerState));
					IAbilitySystemInterface* AbilitySystemProvider = Cast<IAbilitySystemInterface>(PlayerState);
					ASSERT_THAT(IsNotNull(AbilitySystemProvider));
					
					AbilitySystemComponent = AbilitySystemProvider->GetAbilitySystemComponent();
					ASSERT_THAT(IsNotNull(AbilitySystemComponent));
					ASSERT_THAT(AreEqual(AbilitySystemComponent, CombatComponent->GetAbilitySystemComponent()));
					
					const UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(TestPawn->InputComponent);
					ASSERT_THAT(IsNotNull(EnhancedInputComponent));
					ASSERT_THAT(IsFalse(EnhancedInputComponent->GetActionEventBindings().IsEmpty()));

					const FGameplayTag PrimaryInputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));
					PrimaryInputAction = InputConfig->FindAbilityInputActionForTag(PrimaryInputTag);
					ASSERT_THAT(IsNotNull(PrimaryInputAction));

					const UEnhancedPlayerInput* EnhancedPlayerInput = Cast<UEnhancedPlayerInput>(PlayerController->PlayerInput);
					ASSERT_THAT(IsNotNull(EnhancedPlayerInput));
					for (const FEnhancedActionKeyMapping& Mapping : EnhancedPlayerInput->GetEnhancedActionMappingsView())
					{
						if (Mapping.Action == PrimaryInputAction)
						{
							PrimaryInputKey = Mapping.Key;
							break;
						}
					}
					ASSERT_THAT(IsTrue(PrimaryInputKey.IsValid()));

					FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
					Spec.GetDynamicSpecSourceTags().AddTag(PrimaryInputTag);
					GivenHandle = AbilitySystemComponent->GiveAbility(Spec);
					ASSERT_THAT(IsTrue(GivenHandle.IsValid()));
				})
				.Then([this]() {
					PlayerController->InputKey(FInputKeyEventArgs::CreateSimulated(PrimaryInputKey, IE_Pressed, 1.0f));
				})
				.Until([this]() {
					const FGameplayAbilitySpec* FoundSpec = AbilitySystemComponent->FindAbilitySpecFromHandle(GivenHandle);
					return FoundSpec && FoundSpec->IsActive() && FoundSpec->GetPrimaryInstance();
				})
				.Then([this]() {
					FGameplayAbilitySpec* FoundSpec = AbilitySystemComponent->FindAbilitySpecFromHandle(GivenHandle);
					ASSERT_THAT(IsNotNull(FoundSpec));
					ASSERT_THAT(IsTrue(FoundSpec->IsActive()));

					const UTestGameplayAbility* AbilityInstance = Cast<UTestGameplayAbility>(FoundSpec->GetPrimaryInstance());
					ASSERT_THAT(IsNotNull(AbilityInstance));
					ASSERT_THAT(AreEqual(1, AbilityInstance->GetActivationCount()));
				});
		}
	};
}
