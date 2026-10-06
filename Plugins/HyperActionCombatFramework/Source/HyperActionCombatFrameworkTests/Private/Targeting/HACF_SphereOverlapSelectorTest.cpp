#include "CQTest.h"
#include "Components/ActorTestSpawner.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Targeting/HACF_TargetingConfig.h"
#include "Targeting/HACF_TargetingExecutor.h"
#include "Targeting/Processor/HACF_TargetSelector_SphereOverlap.h"

namespace HyperActionCombatFrameworkTests
{
	namespace
	{
		AActor& SpawnCollisionActor(FActorTestSpawner& Spawner, const FVector& Location,
			const ECollisionChannel ObjectType)
		{
			AActor& Actor = Spawner.SpawnActor<AActor>();
			USphereComponent* Collision = NewObject<USphereComponent>(&Actor);
			Actor.AddInstanceComponent(Collision);
			Actor.SetRootComponent(Collision);
			Collision->SetSphereRadius(25.0f);
			Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Collision->SetCollisionObjectType(ObjectType);
			Collision->SetCollisionResponseToAllChannels(ECR_Overlap);
			Collision->RegisterComponent();
			Actor.SetActorLocation(Location);
			return Actor;
		}

		FHACF_TargetingRequest MakeRequest(UHACF_TargetingConfig* Config, AActor* SourceActor,
			const float Range = 300.0f)
		{
			FHACF_TargetingRequest Request;
			Request.Config = Config;
			Request.SourceActor = SourceActor;
			Request.Origin = FVector::ZeroVector;
			Request.bOverrideRange = true;
			Request.RangeOverride = Range;
			return Request;
		}
	}

	TEST_CLASS(FHACF_SphereOverlapSelectorTest, "HyperActionCombat.Tests.Targeting.SphereOverlap")
	{
		FActorTestSpawner Spawner;

		TEST_METHOD(Select_CollectsOverlappingActorsWithinEffectiveRange)
		{
			AActor& Source = SpawnCollisionActor(Spawner, FVector(1000.0, 0.0, 0.0), ECC_WorldDynamic);
			AActor& InRange = SpawnCollisionActor(Spawner, FVector(1100.0, 0.0, 0.0), ECC_Pawn);
			SpawnCollisionActor(Spawner, FVector(1800.0, 0.0, 0.0), ECC_Pawn);
			SpawnCollisionActor(Spawner, FVector(100.0, 0.0, 0.0), ECC_Pawn);

			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->MaxTargets = 10;
			Config->Selectors.Add(NewObject<UHACF_TargetSelector_SphereOverlap>(Config));
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();

			FHACF_TargetingRequest Request = MakeRequest(Config, &Source);
			// 검색 중심은 SourceActor 위치로 자동 결정되지 않으며, 요청의 Origin을 사용한다.
			Request.Origin = Source.GetActorLocation();
			const FHACF_TargetingResult Result = Executor->Execute(Request);
			ASSERT_THAT(IsTrue(Result.IsSuccess()));
			ASSERT_THAT(AreEqual(1, Result.Candidates.Num()));
			ASSERT_THAT(IsTrue(Result.Candidates[0].TargetActor.Get() == &InRange));
			ASSERT_THAT(IsTrue(Result.Candidates[0].TargetLocation.Equals(InRange.GetActorLocation())));
		}

		TEST_METHOD(Select_UsesConfiguredObjectTypes)
		{
			AActor& Source = Spawner.SpawnActor<AActor>();
			AActor& DynamicActor = SpawnCollisionActor(Spawner, FVector(100.0, 0.0, 0.0), ECC_WorldDynamic);
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			UHACF_TargetSelector_SphereOverlap* Selector = NewObject<UHACF_TargetSelector_SphereOverlap>(Config);
			Config->Selectors.Add(Selector);
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();

			ASSERT_THAT(IsTrue(Executor->Execute(MakeRequest(Config, &Source)).Status ==
				EHACF_TargetingResultStatus::NoCandidates));

			Selector->ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
			const FHACF_TargetingResult Result = Executor->Execute(MakeRequest(Config, &Source));
			ASSERT_THAT(IsTrue(Result.IsSuccess()));
			ASSERT_THAT(AreEqual(1, Result.Candidates.Num()));
			ASSERT_THAT(IsTrue(Result.Candidates[0].TargetActor.Get() == &DynamicActor));
		}

		TEST_METHOD(Select_AddsActorOnlyOnceForMultipleCollisionComponents)
		{
			AActor& Source = Spawner.SpawnActor<AActor>();
			AActor& Target = SpawnCollisionActor(Spawner, FVector(100.0, 0.0, 0.0), ECC_Pawn);
			USphereComponent* ExtraCollision = NewObject<USphereComponent>(&Target);
			Target.AddInstanceComponent(ExtraCollision);
			ExtraCollision->SetupAttachment(Target.GetRootComponent());
			ExtraCollision->SetSphereRadius(25.0f);
			ExtraCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			ExtraCollision->SetCollisionObjectType(ECC_Pawn);
			ExtraCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
			ExtraCollision->RegisterComponent();

			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->MaxTargets = 10;
			Config->Selectors.Add(NewObject<UHACF_TargetSelector_SphereOverlap>(Config));
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();
			const FHACF_TargetingResult Result = Executor->Execute(MakeRequest(Config, &Source));

			ASSERT_THAT(IsTrue(Result.IsSuccess()));
			ASSERT_THAT(AreEqual(1, Result.Candidates.Num()));
			ASSERT_THAT(IsTrue(Result.Candidates[0].TargetActor.Get() == &Target));
		}

		TEST_METHOD(Select_RequiresWorldAndObjectTypes)
		{
			AActor& Source = Spawner.SpawnActor<AActor>();
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			UHACF_TargetSelector_SphereOverlap* Selector = NewObject<UHACF_TargetSelector_SphereOverlap>(Config);
			Config->Selectors.Add(Selector);
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();

			ASSERT_THAT(IsTrue(Executor->Execute(MakeRequest(Config, nullptr)).Status ==
				EHACF_TargetingResultStatus::InvalidRequest));

			Selector->ObjectTypes.Reset();
			ASSERT_THAT(IsTrue(Executor->Execute(MakeRequest(Config, &Source)).Status ==
				EHACF_TargetingResultStatus::InvalidRequest));
		}
	};
}
