#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "CQTest.h"
#include "Character/HACF_HeroComponent.h"
#include "Components/MapTestSpawner.h"
#include "Core/HACF_CombatComponent.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystem/Attributes/HACF_AttributeSet.h"
#include "Character/HACF_PawnData.h"
#include "Input/HACF_PawnTestActions.h"
#include "Input/HACF_InputConfig.h"

namespace HyperActionCombatFrameworkTests
{
	// 이 테스트는 #28 수직 슬라이스의 현재 통과 가능한 기반 계약을 보호한다.
	// 실제 피해, 사망, 회피 Cooldown 검증은 대상 더미와 실제 Ability가 추가되는 다음 단계에서 확장한다.
	TEST_CLASS(HACF_CombatVerticalSliceFoundationTest, "HyperActionCombat.Tests.CombatVerticalSlice.Foundation")
	{
		TUniquePtr<FMapTestSpawner> Spawner;

		APawn* PlayerPawn = nullptr;
		APlayerController* PlayerController = nullptr;
		UAbilitySystemComponent* AbilitySystemComponent = nullptr;
		UHACF_CombatComponent* CombatComponent = nullptr;
		UHACF_HeroComponent* HeroComponent = nullptr;
		const UHACF_AttributeSet* CombatAttributes = nullptr;
		const UInputAction* PrimaryInputAction = nullptr;
		TUniquePtr<FHACFPawnTestActions> PawnActions;
		FGameplayAbilitySpecHandle PrimaryAbilityHandle;

		BEFORE_EACH()
		{
			Spawner = MakeUnique<FMapTestSpawner>(TEXT("/Game/Maps"), TEXT("L_CombatTest"));
			Spawner->AddWaitUntilLoadedCommand(TestRunner);
		}

		AFTER_EACH()
		{
			if (PawnActions)
			{
				PawnActions->StopAllActions();
			}
		}

		TEST_METHOD(CombatMap_ProvidesPlayerAbilitySystemAndPrimaryInputContract)
		{
			TestCommandBuilder
				.StartWhen([this]() {
					APawn* Pawn = Spawner->FindFirstPlayerPawn();
					if (!Pawn || !Pawn->InputComponent)
					{
						return false;
					}

					APlayerController* Controller = Cast<APlayerController>(Pawn->GetController());
					APlayerState* PlayerState = Controller ? Controller->GetPlayerState<APlayerState>() : nullptr;
					IAbilitySystemInterface* AbilitySystemProvider = Cast<IAbilitySystemInterface>(PlayerState);
					UAbilitySystemComponent* ASC = AbilitySystemProvider ? AbilitySystemProvider->GetAbilitySystemComponent() : nullptr;
					const UHACF_AttributeSet* Attributes = ASC ? ASC->GetSet<UHACF_AttributeSet>() : nullptr;

					return Pawn->FindComponentByClass<UHACF_CombatComponent>()
						&& Pawn->FindComponentByClass<UHACF_HeroComponent>()
						&& ASC && Attributes && Attributes->GetMaxHealth() > 0.f;
				})
				.Then([this]() {
					PlayerPawn = Spawner->FindFirstPlayerPawn();
					ASSERT_THAT(IsNotNull(PlayerPawn));

					PlayerController = Cast<APlayerController>(PlayerPawn->GetController());
					ASSERT_THAT(IsNotNull(PlayerController));

					CombatComponent = PlayerPawn->FindComponentByClass<UHACF_CombatComponent>();
					HeroComponent = PlayerPawn->FindComponentByClass<UHACF_HeroComponent>();
					ASSERT_THAT(IsNotNull(CombatComponent));
					ASSERT_THAT(IsNotNull(HeroComponent));

					APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>();
					ASSERT_THAT(IsNotNull(PlayerState));

					IAbilitySystemInterface* AbilitySystemProvider = Cast<IAbilitySystemInterface>(PlayerState);
					ASSERT_THAT(IsNotNull(AbilitySystemProvider));

					AbilitySystemComponent = AbilitySystemProvider->GetAbilitySystemComponent();
					ASSERT_THAT(IsNotNull(AbilitySystemComponent));
					ASSERT_THAT(AreEqual(AbilitySystemComponent, CombatComponent->GetAbilitySystemComponent()));

					CombatAttributes = AbilitySystemComponent->GetSet<UHACF_AttributeSet>();
					ASSERT_THAT(IsNotNull(CombatAttributes));
					ASSERT_THAT(IsTrue(CombatAttributes->GetMaxHealth() > 0.f));
					ASSERT_THAT(IsTrue(CombatAttributes->GetHealth() > 0.f));

					const UHACF_PawnData* PawnData = Cast<UHACF_PawnData>(StaticLoadObject(
						UHACF_PawnData::StaticClass(), nullptr,
						TEXT("/Script/HyperActionCombatFramework.HACF_PawnData'/Game/Data/CombatPawnData.CombatPawnData'")));
					ASSERT_THAT(IsNotNull(PawnData));
					ASSERT_THAT(IsNotNull(PawnData->InputConfig));

					const FGameplayTag PrimaryInputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));
					PrimaryInputAction = PawnData->InputConfig->FindAbilityInputActionForTag(PrimaryInputTag);
					ASSERT_THAT(IsNotNull(PrimaryInputAction));

					const UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerPawn->InputComponent);
					ASSERT_THAT(IsNotNull(EnhancedInputComponent));
					ASSERT_THAT(IsFalse(EnhancedInputComponent->GetActionEventBindings().IsEmpty()));

					const UEnhancedPlayerInput* EnhancedPlayerInput = Cast<UEnhancedPlayerInput>(PlayerController->PlayerInput);
					ASSERT_THAT(IsNotNull(EnhancedPlayerInput));
					
					FKey PrimaryInputKey;
					for (const FEnhancedActionKeyMapping& Mapping : EnhancedPlayerInput->GetEnhancedActionMappingsView())
					{
						if (Mapping.Action == PrimaryInputAction)
						{
							PrimaryInputKey = Mapping.Key;
							break;
						}
					}
					ASSERT_THAT(IsTrue(PrimaryInputKey.IsValid()));

					for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
					{
						if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(PrimaryInputTag))
						{
							PrimaryAbilityHandle = AbilitySpec.Handle;
							break;
						}
					}
					ASSERT_THAT(IsTrue(PrimaryAbilityHandle.IsValid()));
					
					PawnActions = MakeUnique<FHACFPawnTestActions>(PlayerPawn);
				})
				.Then([this]() {
					PawnActions->PressAbilityAction(PrimaryInputAction);
				})
				.Until([this]() {
					const FGameplayAbilitySpec* PrimaryAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(PrimaryAbilityHandle);
					return PrimaryAbilitySpec && PrimaryAbilitySpec->InputPressed;
				})
				.Then([this]() {
					const FGameplayAbilitySpec* PrimaryAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(PrimaryAbilityHandle);
					ASSERT_THAT(IsNotNull(PrimaryAbilitySpec));
					ASSERT_THAT(IsTrue(PrimaryAbilitySpec->InputPressed));
				});
		}
	};
}
