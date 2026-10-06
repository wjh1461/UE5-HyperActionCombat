#include "CQTest.h"
#include "Components/ActorTestSpawner.h"
#include "GameFramework/Actor.h"
#include "Targeting/HACF_TargetingConfig.h"
#include "Targeting/Processor/HACF_TargetScorer_AimAndDistance.h"

namespace HyperActionCombatFrameworkTests
{
	namespace
	{
		FHACF_TargetingCandidate MakeAimAndDistanceCandidate(const FVector& Location, const float InitialScore = 0.0f)
		{
			FHACF_TargetingCandidate Candidate;
			Candidate.TargetCategory = EHACF_TargetCategory::Location;
			Candidate.TargetLocation = Location;
			Candidate.Score = InitialScore;
			return Candidate;
		}

		FHACF_TargetingExecutionContext MakeContext(UHACF_TargetingConfig* Config)
		{
			FHACF_TargetingRequest Request;
			Request.Config = Config;
			Request.Origin = FVector::ZeroVector;
			Request.AimDirection = FVector::ForwardVector;
			Request.bOverrideRange = true;
			Request.RangeOverride = 100.0f;

			FHACF_TargetingExecutionContext Context;
			Context.Initialize(Request);
			return Context;
		}
	}

	TEST_CLASS(FHACF_AimAndDistanceScorerTest, "HyperActionCombat.Tests.Targeting.AimAndDistance")
	{
		FActorTestSpawner Spawner;

		TEST_METHOD(Score_NormalizesAimAndDistanceAndAccumulatesExistingScore)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			FHACF_TargetingExecutionContext Context = MakeContext(Config);
			Context.Request.AimDirection = FVector(2.0, 0.0, 0.0);
			Context.Candidates = {
				MakeAimAndDistanceCandidate(FVector(50.0, 0.0, 0.0), 2.0f),
				MakeAimAndDistanceCandidate(FVector(0.0, 100.0, 0.0)),
				MakeAimAndDistanceCandidate(FVector(-100.0, 0.0, 0.0)),
				MakeAimAndDistanceCandidate(FVector(50.0, 50.0 * FMath::Sqrt(3.0), 0.0)),
				MakeAimAndDistanceCandidate(FVector(200.0, 0.0, 0.0))
			};

			const UHACF_TargetScorer_AimAndDistance* Scorer = NewObject<UHACF_TargetScorer_AimAndDistance>();
			ASSERT_THAT(IsTrue(Scorer->Score(Context) == EHACF_TargetingProcessResult::Success));
			ASSERT_THAT(IsNear(Context.Candidates[0].Score, 2.85f, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(Context.Candidates[1].Score, 0.35f, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(Context.Candidates[2].Score, 0.0f, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(Context.Candidates[3].Score, 0.525f, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(Context.Candidates[4].Score, 0.7f, KINDA_SMALL_NUMBER));
		}

		TEST_METHOD(Score_AddsPreferredActorBonus)
		{
			AActor& PreferredActor = Spawner.SpawnActor<AActor>();
			AActor& OtherActor = Spawner.SpawnActor<AActor>();
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Actor;
			FHACF_TargetingExecutionContext Context = MakeContext(Config);
			Context.Request.PreferredTarget = &PreferredActor;

			for (AActor* Actor : {&PreferredActor, &OtherActor})
			{
				FHACF_TargetingCandidate& Candidate = Context.Candidates.AddDefaulted_GetRef();
				Candidate.TargetCategory = EHACF_TargetCategory::Actor;
				Candidate.TargetActor = Actor;
				Candidate.TargetLocation = FVector(50.0, 0.0, 0.0);
			}

			const UHACF_TargetScorer_AimAndDistance* Scorer = NewObject<UHACF_TargetScorer_AimAndDistance>();
			ASSERT_THAT(IsTrue(Scorer->Score(Context) == EHACF_TargetingProcessResult::Success));
			ASSERT_THAT(IsNear(Context.Candidates[0].Score, 0.95f, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(Context.Candidates[1].Score, 0.85f, KINDA_SMALL_NUMBER));
		}

		TEST_METHOD(Score_AppliesConfiguredAimAndDistanceWeights)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			FHACF_TargetingExecutionContext Context = MakeContext(Config);
			Context.Candidates.Add(MakeAimAndDistanceCandidate(FVector(100.0, 0.0, 0.0)));

			UHACF_TargetScorer_AimAndDistance* Scorer = NewObject<UHACF_TargetScorer_AimAndDistance>();
			Scorer->AimWeight = 2.0f;
			Scorer->DistanceWeight = 1.0f;

			ASSERT_THAT(IsTrue(Scorer->Score(Context) == EHACF_TargetingProcessResult::Success));
			ASSERT_THAT(IsNear(Context.Candidates[0].Score, 2.0f / 3.0f, KINDA_SMALL_NUMBER));
		}

		TEST_METHOD(Score_ZeroWeightSumFailsRequestAndClearsCandidates)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			FHACF_TargetingExecutionContext Context = MakeContext(Config);
			Context.Candidates.Add(MakeAimAndDistanceCandidate(FVector(50.0, 0.0, 0.0)));

			UHACF_TargetScorer_AimAndDistance* Scorer = NewObject<UHACF_TargetScorer_AimAndDistance>();
			Scorer->AimWeight = 0.0f;
			Scorer->DistanceWeight = 0.0f;

			ASSERT_THAT(IsTrue(Scorer->Score(Context) == EHACF_TargetingProcessResult::Failed));
			ASSERT_THAT(IsTrue(Context.FailureStatus == EHACF_TargetingResultStatus::InvalidRequest));
			ASSERT_THAT(IsTrue(Context.Candidates.IsEmpty()));
		}

		TEST_METHOD(Score_InvalidAimDirectionFailsRequest)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			FHACF_TargetingExecutionContext Context = MakeContext(Config);
			Context.Request.AimDirection = FVector::ZeroVector;
			Context.Candidates.Add(MakeAimAndDistanceCandidate(FVector(50.0, 0.0, 0.0)));

			const UHACF_TargetScorer_AimAndDistance* Scorer = NewObject<UHACF_TargetScorer_AimAndDistance>();
			ASSERT_THAT(IsTrue(Scorer->Score(Context) == EHACF_TargetingProcessResult::Failed));
			ASSERT_THAT(IsTrue(Context.FailureStatus == EHACF_TargetingResultStatus::InvalidRequest));
			ASSERT_THAT(IsTrue(Context.Candidates.IsEmpty()));
		}
	};
}
