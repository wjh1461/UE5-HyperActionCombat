#include "CQTest.h"
#include "Character/HACF_PawnData.h"
#include "Components/ActorTestSpawner.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Core/HACF_CombatComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "HACF_GameplayTags.h"
#include "InitState/HACF_InitStateTestHeroComponent.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(HACF_InitState_HeroTest, "HyperActionCombat.Tests.InitState.HeroComponent")
	{
		FActorTestSpawner Spawner;
		APawn* Pawn = nullptr;
		UHACF_InitStateTestHeroComponent* HeroComponent = nullptr;
		UHACF_PawnData* PawnData = nullptr;
		UGameFrameworkComponentManager* Manager = nullptr;

		BEFORE_EACH()
		{
			Spawner.InitializeGameSubsystems();

			Pawn = &Spawner.SpawnActor<APawn>();
			HeroComponent = NewObject<UHACF_InitStateTestHeroComponent>(Pawn, TEXT("HeroComponent"));
			Pawn->AddInstanceComponent(HeroComponent);
			PawnData = NewObject<UHACF_PawnData>(Pawn, TEXT("PawnData"));

			Manager = Spawner.GetWorld().GetGameInstance()->GetSubsystem<UGameFrameworkComponentManager>();

			ASSERT_THAT(IsNotNull(Pawn));
			ASSERT_THAT(IsNotNull(HeroComponent));
			ASSERT_THAT(IsNotNull(PawnData));
			ASSERT_THAT(IsNotNull(Manager));
		}

		AFTER_EACH()
		{
			if (IsValid(Pawn))
			{
				Pawn->Destroy();
				Pawn = nullptr;
			}
		}

		UHACF_CombatComponent* AddCombatComponent()
		{
			UHACF_CombatComponent* CombatComponent = NewObject<UHACF_CombatComponent>(Pawn, TEXT("CombatComponent"));
			Pawn->AddInstanceComponent(CombatComponent);
			return CombatComponent;
		}

		TEST_METHOD(HeroComponent_WithPawn_CanEnterSpawned)
		{
			const bool bCanEnterSpawned = HeroComponent->CanChangeInitState(
				Manager,
				FGameplayTag(),
				HACF_GameplayTags::InitState_Spawned);

			ASSERT_THAT(IsTrue(bCanEnterSpawned));
		}

		TEST_METHOD(HeroComponent_WithoutPawnData_CannotEnterDataAvailable)
		{
			AddCombatComponent();

			const bool bCanEnterDataAvailable = HeroComponent->CanChangeInitState(
				Manager,
				HACF_GameplayTags::InitState_Spawned,
				HACF_GameplayTags::InitState_DataAvailable);

			ASSERT_THAT(IsFalse(bCanEnterDataAvailable));
		}

		TEST_METHOD(HeroComponent_WithoutCombatComponent_CannotEnterDataAvailable)
		{
			HeroComponent->SetPawnDataForTest(PawnData);

			const bool bCanEnterDataAvailable = HeroComponent->CanChangeInitState(
				Manager,
				HACF_GameplayTags::InitState_Spawned,
				HACF_GameplayTags::InitState_DataAvailable);

			ASSERT_THAT(IsFalse(bCanEnterDataAvailable));
		}

		TEST_METHOD(HeroComponent_WithPawnDataAndCombatComponent_CanEnterDataAvailable)
		{
			AddCombatComponent();
			HeroComponent->SetPawnDataForTest(PawnData);

			const bool bCanEnterDataAvailable = HeroComponent->CanChangeInitState(
				Manager,
				HACF_GameplayTags::InitState_Spawned,
				HACF_GameplayTags::InitState_DataAvailable);

			ASSERT_THAT(IsTrue(bCanEnterDataAvailable));
		}

		TEST_METHOD(HeroComponent_WaitsForCombatComponentInitStates)
		{
			UHACF_CombatComponent* CombatComponent = AddCombatComponent();
			HeroComponent->SetPawnDataForTest(PawnData);

			Manager->RegisterFeatureImplementer(Pawn, UHACF_CombatComponent::NAME_ActorFeatureName, CombatComponent);

			ASSERT_THAT(IsFalse(HeroComponent->CanChangeInitState(
				Manager,
				HACF_GameplayTags::InitState_DataAvailable,
				HACF_GameplayTags::InitState_DataInitialized)));

			Manager->ChangeFeatureInitState(Pawn, UHACF_CombatComponent::NAME_ActorFeatureName, CombatComponent,
				HACF_GameplayTags::InitState_DataInitialized);

			ASSERT_THAT(IsTrue(HeroComponent->CanChangeInitState(
				Manager,
				HACF_GameplayTags::InitState_DataAvailable,
				HACF_GameplayTags::InitState_DataInitialized)));
			ASSERT_THAT(IsFalse(HeroComponent->CanChangeInitState(
				Manager,
				HACF_GameplayTags::InitState_DataInitialized,
				HACF_GameplayTags::InitState_GameplayReady)));

			Manager->ChangeFeatureInitState(Pawn, UHACF_CombatComponent::NAME_ActorFeatureName, CombatComponent,
				HACF_GameplayTags::InitState_GameplayReady);

			ASSERT_THAT(IsTrue(HeroComponent->CanChangeInitState(
				Manager,
				HACF_GameplayTags::InitState_DataInitialized,
				HACF_GameplayTags::InitState_GameplayReady)));
		}
	};
}
