#include "AbilitySystemComponent.h"
#include "CQTest.h"
#include "Abilities/TestGameplayAbility.h"
#include "Character/HACF_HeroComponent.h"
#include "Character/HACF_PawnData.h"
#include "Components/MapTestSpawner.h"
#include "Core/HACF_CombatComponent.h"
#include "Input/HACF_PawnTestActions.h"
#include "Input/HACF_InputConfig.h"

namespace HyperActionCombatFrameworkTests
{
	namespace AssetPaths
	{
		constexpr const TCHAR* TestInputPawnData =
			TEXT("/Script/HyperActionCombatFramework.HACF_PawnData'/Game/Test/Data/CombatPawnData_TestInput.CombatPawnData_TestInput'");
	}

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

			PawnData = Cast<UHACF_PawnData>(StaticLoadObject(
				UHACF_PawnData::StaticClass(), nullptr, AssetPaths::TestInputPawnData));
			ASSERT_THAT(IsNotNull(PawnData));

			InputConfig = PawnData ? PawnData->InputConfig : nullptr;
			ASSERT_THAT(IsNotNull(InputConfig));
		}

		AFTER_EACH()
		{
			if (PawnActions)
			{
				PawnActions->StopAllActions();
			}

			if (HeroComponent)
			{
				HeroComponent->DestroyComponent();
				HeroComponent = nullptr;
			}

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

		TEST_METHOD(SimulateInput_AllAbilityActions_TriggerMappedTags)
		{
			SetupTestEnvironment();
			VerifyInputTriggers(FName("HACF.Input.Action.Primary"));
			VerifyInputTriggers(FName("HACF.Input.Action.Ability.Dodge"));
			VerifyInputTriggers(FName("HACF.Input.Action.Ability.Normal"));
			VerifyInputTriggers(FName("HACF.Input.Action.Ability.Ultimate"));
		}

		TEST_METHOD(SimulateInput_AllAbilityActions_ReleaseMappedTags)
		{
			SetupTestEnvironment();
			VerifyInputReleased(FName("HACF.Input.Action.Primary"));
			VerifyInputReleased(FName("HACF.Input.Action.Ability.Dodge"));
			VerifyInputReleased(FName("HACF.Input.Action.Ability.Normal"));
			VerifyInputReleased(FName("HACF.Input.Action.Ability.Ultimate"));
		}
	};

}
