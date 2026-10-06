#include "CQTest.h"
#include "Components/ActorTestSpawner.h"
#include "GameFramework/Pawn.h"
#include "Targeting/HACF_TargetingTestService.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(FHACF_CombatTargetingServiceTest, "HyperActionCombat.Tests.Targeting.CombatService")
	{
		FActorTestSpawner Spawner;
		APawn* Pawn = nullptr;
		UHACF_TargetingTestCombatComponent* CombatComponent = nullptr;

		BEFORE_EACH()
		{
			Pawn = &Spawner.SpawnActor<APawn>();
			CombatComponent = NewObject<UHACF_TargetingTestCombatComponent>(Pawn, TEXT("CombatComponent"));
			Pawn->AddInstanceComponent(CombatComponent);

			ASSERT_THAT(IsNotNull(Pawn));
			ASSERT_THAT(IsNotNull(CombatComponent));
		}

		TEST_METHOD(ExecuteTargeting_WithoutDiscoveredService_ReturnsServiceUnavailable)
		{
			ASSERT_THAT(IsFalse(CombatComponent->HasTargetingService()));

			const FHACF_TargetingResult Result = CombatComponent->ExecuteTargeting(FHACF_TargetingRequest());

			ASSERT_THAT(IsTrue(Result.Status == EHACF_TargetingResultStatus::ServiceUnavailable));
		}

		TEST_METHOD(RefreshTargetingService_AfterServiceAdded_CachesService)
		{
			ASSERT_THAT(IsFalse(CombatComponent->RefreshTargetingService()));
			ASSERT_THAT(IsFalse(CombatComponent->HasTargetingService()));

			UHACF_TargetingTestService* Service = NewObject<UHACF_TargetingTestService>(Pawn, TEXT("TargetingService"));
			Pawn->AddInstanceComponent(Service);

			ASSERT_THAT(IsTrue(CombatComponent->RefreshTargetingService()));
			ASSERT_THAT(IsTrue(CombatComponent->HasTargetingService()));
		}

		TEST_METHOD(BeginPlay_WithServiceOnPawn_DiscoversServiceAutomatically)
		{
			Spawner.InitializeGameSubsystems();

			UHACF_TargetingTestService* Service = NewObject<UHACF_TargetingTestService>(Pawn, TEXT("TargetingService"));
			Pawn->AddInstanceComponent(Service);
			Service->RegisterComponent();
			CombatComponent->RegisterComponent();

			ASSERT_THAT(IsFalse(CombatComponent->HasTargetingService()));

			CombatComponent->InvokeBeginPlay();

			ASSERT_THAT(IsTrue(CombatComponent->HasTargetingService()));
		}

		TEST_METHOD(ExecuteTargeting_WithCachedService_FillsSourceAndDefaultRequestOwner)
		{
			UHACF_TargetingTestService* Service = NewObject<UHACF_TargetingTestService>(Pawn, TEXT("TargetingService"));
			Pawn->AddInstanceComponent(Service);
			Service->ResultToReturn.Reset(EHACF_TargetingResultStatus::NoCandidates);

			ASSERT_THAT(IsTrue(CombatComponent->RefreshTargetingService()));

			FHACF_TargetingRequest Request;
			const FHACF_TargetingResult Result = CombatComponent->ExecuteTargeting(Request);

			ASSERT_THAT(IsTrue(Result.Status == EHACF_TargetingResultStatus::NoCandidates));
			ASSERT_THAT(AreEqual(1, Service->ExecuteCount));
			ASSERT_THAT(AreEqual(Cast<AActor>(Pawn), Service->LastRequest.SourceActor.Get()));
			ASSERT_THAT(AreEqual(Cast<UObject>(CombatComponent), Service->LastRequest.RequestOwner.Get()));
		}
	};
}
