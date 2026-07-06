#include "AbilitySystemComponent.h"
#include "CQTest.h"
#include "Abilities/TestGameplayAbility.h"
#include "Character/HACF_HeroComponent.h"
#include "Components/MapTestSpawner.h"
#include "Input/HACF_InputConfig.h"
#include "Components/InputTestActions.h"
#include "Core/HACF_CombatComponent.h"
#include "GameFramework/Pawn.h"

namespace HyperActionCombatFrameworkTests
{
	// 커스텀 Test Action 구조체 정의
	struct FHACFAbilityTestAction : public FTestAction
	{
		FHACFAbilityTestAction(const UInputAction* Action)
		{
			InputActionName = Action->GetName();
			InputActionValue = FInputActionValue(true); 
		}
	};
	
	// 입력 테스트 동작 래핑 클래스
	class FHACFPawnTestActions : public FInputTestActions
	{
	public:
		explicit FHACFPawnTestActions(APawn* Pawn) : FInputTestActions(Pawn) {}

		void PressAbilityAction(const UInputAction* Action)
		{
			PerformAction(FHACFAbilityTestAction(Action));
		}
	};
	
	// 테스트 클래스 내부를 볼 수 있게 하는 접근자
	class UHACF_HeroComponent_Accessor : public UHACF_HeroComponent
	{
	public:
		using UHACF_HeroComponent::BeginPlay;
		using UHACF_HeroComponent::InitializePlayerInput;
		
		using UHACF_HeroComponent::CombatComponent;
		using UHACF_HeroComponent::InputConfig;
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
		UHACF_InputConfig* InputConfig{nullptr};
		APawn* TestPawn{nullptr};
		
		UAbilitySystemComponent* AbilitySystemComponent;
		UHACF_CombatComponent_Accessor* CombatComponent{nullptr};
		UHACF_HeroComponent_Accessor* HeroComponent{nullptr};
		
		TUniquePtr<FHACFPawnTestActions> PawnActions;
		
		FGameplayAbilitySpecHandle GivenHandle;

		BEFORE_EACH()
		{
			Spawner = FMapTestSpawner::CreateFromTempLevel(TestCommandBuilder);
			Spawner->AddWaitUntilLoadedCommand(TestRunner);

			InputConfig = Cast<UHACF_InputConfig>(StaticLoadObject(UHACF_InputConfig::StaticClass(), nullptr, 
				TEXT("/Script/HyperActionCombatFramework.HACF_InputConfig'/Game/Input/InputData_CombatHero.InputData_CombatHero'")
			));
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
					CombatComponent->InitializeWithASC(AbilitySystemComponent);

					HeroComponent = NewObject<UHACF_HeroComponent_Accessor>(TestPawn);
					HeroComponent->RegisterComponent();
					
					HeroComponent->CombatComponent = CombatComponent;
					HeroComponent->InputConfig = InputConfig;
					HeroComponent->InitializePlayerInput(TestPawn->GetController()->InputComponent);
				});
		}

		void VerifyInputTriggers(FName TagName)
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
				.Then([this, TagName]() {
					ASSERT_THAT(IsTrue(CombatComponent->InputPressedSpecHandles.Contains(GivenHandle)));
					ASSERT_THAT(IsTrue(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));
				});
		}

		void VerifyInputReleased(FName TagName)
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
				.Then([this]() {
					PawnActions->StopAllActions();
				})
				.Then([this, TagName]() {
					ASSERT_THAT(IsFalse(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));
					ASSERT_THAT(IsTrue(CombatComponent->InputReleasedSpecHandles.Contains(GivenHandle)));
				});
		}

		// 1. PrimaryAttack
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

		// 2. SecondaryDodge
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

		// 3. AbilityNormal
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

		// 4. AbilityUltimate
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
}
