#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
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
#include "Input/HACF_InputConfig.h"

namespace HyperActionCombatFrameworkTests
{
	namespace AssetPaths
	{
		constexpr const TCHAR* CombatPawnData =
			TEXT("/Script/HyperActionCombatFramework.HACF_PawnData'/Game/Data/CombatPawnData.CombatPawnData'");
		constexpr const TCHAR* TestContentRoot = TEXT("/Game/Test/");
	}

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
		FGameplayAbilitySpecHandle PrimaryAbilityHandle;
		FString PrimaryAbilityClassPath;
		FKey PrimaryInputKey;
		FDelegateHandle PrimaryAbilityActivatedDelegateHandle;
		bool bPrimaryAbilityActivated = false;

		BEFORE_EACH()
		{
			Spawner = MakeUnique<FMapTestSpawner>(TEXT("/Game/Test/Maps"), TEXT("L_CQ_CombatVerticalSlice"));
			Spawner->AddWaitUntilLoadedCommand(TestRunner);
		}

		AFTER_EACH()
		{
			if (AbilitySystemComponent && PrimaryAbilityActivatedDelegateHandle.IsValid())
			{
				AbilitySystemComponent->AbilityActivatedCallbacks.Remove(PrimaryAbilityActivatedDelegateHandle);
				PrimaryAbilityActivatedDelegateHandle.Reset();
			}

			if (PlayerController && PrimaryInputKey.IsValid())
			{
				PlayerController->InputKey(
					FInputKeyEventArgs::CreateSimulated(PrimaryInputKey, IE_Released, 0.0f));
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
						UHACF_PawnData::StaticClass(), nullptr, AssetPaths::CombatPawnData));
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
							PrimaryAbilityClassPath = AbilitySpec.Ability
								? AbilitySpec.Ability->GetClass()->GetPathName()
								: FString();
							break;
						}
					}
					ASSERT_THAT(IsTrue(PrimaryAbilityHandle.IsValid()));
					ASSERT_THAT(IsFalse(PrimaryAbilityClassPath.IsEmpty()));
					ASSERT_THAT(IsFalse(PrimaryAbilityClassPath.StartsWith(AssetPaths::TestContentRoot)));

					PrimaryAbilityActivatedDelegateHandle = AbilitySystemComponent->AbilityActivatedCallbacks.AddLambda(
						[this](UGameplayAbility* ActivatedAbility)
						{
							if (ActivatedAbility &&
								ActivatedAbility->GetCurrentAbilitySpecHandle() == PrimaryAbilityHandle)
							{
								bPrimaryAbilityActivated = true;
							}
						});
				})
				.Then([this]() {
					PlayerController->InputKey(
						FInputKeyEventArgs::CreateSimulated(PrimaryInputKey, IE_Pressed, 1.0f));
				})
				.Until([this]() {
					return bPrimaryAbilityActivated;
				})
				.Then([this]() {
					ASSERT_THAT(IsTrue(bPrimaryAbilityActivated));
				});
		}
	};
}
