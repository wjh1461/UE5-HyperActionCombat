#include "AbilitySystemComponent.h"
#include "CQTest.h"
#include "Components/ActorTestSpawner.h"
#include "Core/HACF_CombatComponent.h"
#include "GameFramework/Pawn.h"
#include "HACF_GameplayTags.h"
#include "InitState/HACF_InitStateTestPlayerState.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(HACF_InitState_CombatTest, "HyperActionCombat.Tests.InitState.CombatComponent")
	{
		FActorTestSpawner Spawner;
		APawn* Pawn = nullptr;
		UHACF_CombatComponent* CombatComponent = nullptr;

		BEFORE_EACH()
		{
			Pawn = &Spawner.SpawnActor<APawn>();
			CombatComponent = NewObject<UHACF_CombatComponent>(Pawn, TEXT("CombatComponent"));
			Pawn->AddInstanceComponent(CombatComponent);

			ASSERT_THAT(IsNotNull(Pawn));
			ASSERT_THAT(IsNotNull(CombatComponent));
		}

		// 1. Spawned
		TEST_METHOD(CombatComponent_WithPawn_CanEnterSpawned)
		{
			const bool bCanEnterSpawned = CombatComponent->CanChangeInitState(
				nullptr,
				FGameplayTag(),
				HACF_GameplayTags::InitState_Spawned);

			ASSERT_THAT(IsTrue(bCanEnterSpawned));
		}

		// 2-1. DataAvailable 진입 실패
		TEST_METHOD(CombatComponent_WithoutPlayerState_CannotEnterDataAvailable)
		{
			const bool bCanEnterDataAvailable = CombatComponent->CanChangeInitState(
				nullptr,
				HACF_GameplayTags::InitState_Spawned,
				HACF_GameplayTags::InitState_DataAvailable);

			ASSERT_THAT(IsFalse(bCanEnterDataAvailable));
		}

		// 2-2. DataAvailable 진입 성공
		TEST_METHOD(CombatComponent_WithPlayerStateASC_CanEnterDataAvailable)
		{
			AHACF_InitStateTestPlayerState& PlayerState = Spawner.SpawnActor<AHACF_InitStateTestPlayerState>();
			Pawn->SetPlayerState(&PlayerState);

			const bool bCanEnterDataAvailable = CombatComponent->CanChangeInitState(
				nullptr,
				HACF_GameplayTags::InitState_Spawned,
				HACF_GameplayTags::InitState_DataAvailable);

			ASSERT_THAT(IsTrue(bCanEnterDataAvailable));
		}

		// 3. DataInitialized 진입, GameplayReady 진입
		TEST_METHOD(CombatComponent_EnteringDataInitialized_InitializesASCActorInfo_AndEnablesGameplayReady)
		{
			AHACF_InitStateTestPlayerState& PlayerState = Spawner.SpawnActor<AHACF_InitStateTestPlayerState>();
			Pawn->SetPlayerState(&PlayerState);

			CombatComponent->HandleChangeInitState(
				nullptr,
				HACF_GameplayTags::InitState_DataAvailable,
				HACF_GameplayTags::InitState_DataInitialized);

			UAbilitySystemComponent* ASC = PlayerState.GetAbilitySystemComponent();
			ASSERT_THAT(AreEqual(ASC, CombatComponent->GetAbilitySystemComponent()));
			ASSERT_THAT(AreEqual(Cast<AActor>(&PlayerState), ASC->GetOwnerActor()));
			ASSERT_THAT(AreEqual(Cast<AActor>(Pawn), ASC->GetAvatarActor()));

			const bool bCanEnterGameplayReady = CombatComponent->CanChangeInitState(
				nullptr,
				HACF_GameplayTags::InitState_DataInitialized,
				HACF_GameplayTags::InitState_GameplayReady);
			ASSERT_THAT(IsTrue(bCanEnterGameplayReady));
		}
	};
}
