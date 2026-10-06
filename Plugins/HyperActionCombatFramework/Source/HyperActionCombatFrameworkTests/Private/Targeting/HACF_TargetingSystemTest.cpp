#include "CQTest.h"
#include "Components/ActorTestSpawner.h"
#include "GameFramework/Actor.h"
#include "Targeting/HACF_TargetingConfig.h"
#include "Targeting/HACF_TargetingExecutor.h"
#include "Targeting/Processor/HACF_TargetFilter_ActorForwardSectorAndRange.h"
#include "Targeting/HACF_TargetingTestProcessors.h"

#include <limits>

namespace HyperActionCombatFrameworkTests
{
	namespace
	{
		FHACF_TargetingCandidate MakeLocationCandidate(const FVector& Location, const float Score = 0.0f,
			const bool bHasDirection = false, const FVector& Direction = FVector::ZeroVector)
		{
			FHACF_TargetingCandidate Candidate;
			Candidate.TargetCategory = EHACF_TargetCategory::Location;
			Candidate.TargetLocation = Location;
			Candidate.Score = Score;
			Candidate.bHasDirection = bHasDirection;
			Candidate.TargetDirection = Direction;
			return Candidate;
		}

		FHACF_TargetingCandidate MakeActorCandidate(AActor& Actor, const float Score = 0.0f)
		{
			FHACF_TargetingCandidate Candidate;
			Candidate.TargetActor = &Actor;
			Candidate.TargetLocation = Actor.GetActorLocation();
			Candidate.Score = Score;
			return Candidate;
		}

		FHACF_TargetingRequest MakeRequest(UHACF_TargetingConfig* Config)
		{
			FHACF_TargetingRequest Request;
			Request.Config = Config;
			return Request;
		}
	}

	TEST_CLASS(FHACF_TargetingSystemTest, "HyperActionCombat.Tests.Targeting")
	{
		FActorTestSpawner Spawner;

		TEST_METHOD(Execute_CrossSelectorDuplicatesPreserveFirstMetadataBeforeLimit)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->MaxTargets = 2;
			AActor& FirstActor = Spawner.SpawnActor<AActor>();
			AActor& SecondActor = Spawner.SpawnActor<AActor>();
			UHACF_TargetingTestSelector* First = NewObject<UHACF_TargetingTestSelector>(Config);
			UHACF_TargetingTestSelector* Second = NewObject<UHACF_TargetingTestSelector>(Config);
			FHACF_TargetingCandidate Original = MakeActorCandidate(FirstActor, 5.0f);
			Original.TargetLocation = FVector(10.0, 20.0, 30.0);
			Original.bHasDirection = true;
			Original.TargetDirection = FVector::RightVector;
			Original.bHasHitResult = true;
			Original.HitResult.ImpactPoint = Original.TargetLocation;
			FHACF_TargetingCandidate InvalidFirst = MakeActorCandidate(FirstActor, std::numeric_limits<float>::infinity());
			First->Behavior.CandidatesToAdd = {InvalidFirst, Original};
			Second->Behavior.CandidatesToAdd = {MakeActorCandidate(FirstActor, 100.0f),
				MakeLocationCandidate(FVector(100.0, 0.0, 0.0)), MakeActorCandidate(SecondActor, 5.0f)};
			Config->Selectors = {First, Second};
			const FHACF_TargetingResult Result = NewObject<UHACF_TargetingExecutor>()->Execute(MakeRequest(Config));
			ASSERT_THAT(AreEqual(2, Result.Candidates.Num()));
			ASSERT_THAT(IsTrue(Result.Candidates[0].TargetActor.Get() == &FirstActor));
			ASSERT_THAT(IsTrue(Result.Candidates[1].TargetActor.Get() == &SecondActor));
			ASSERT_THAT(IsTrue(Result.Candidates[0].TargetLocation.Equals(Original.TargetLocation)));
			ASSERT_THAT(IsTrue(Result.Candidates[0].bHasDirection));
			ASSERT_THAT(IsTrue(Result.Candidates[0].TargetDirection.Equals(Original.TargetDirection)));
			ASSERT_THAT(IsTrue(Result.Candidates[0].bHasHitResult));
			ASSERT_THAT(IsTrue(Result.Candidates[0].HitResult.ImpactPoint.Equals(Original.HitResult.ImpactPoint)));
			ASSERT_THAT(IsTrue(FMath::IsNearlyEqual(5.0f, Result.Candidates[0].Score)));
			ASSERT_THAT(AreEqual(uint32(1), Result.Candidates[0].StableSortIndex));
			ASSERT_THAT(AreEqual(uint32(4), Result.Candidates[1].StableSortIndex));
		}

		TEST_METHOD(Execute_ExclusionsApplyWithoutFiltersAndRespectSourceToggle)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->MaxTargets = 4;
			AActor& Source = Spawner.SpawnActor<AActor>();
			AActor& Excluded = Spawner.SpawnActor<AActor>();
			AActor& Included = Spawner.SpawnActor<AActor>();
			UHACF_TargetingTestSelector* Selection = NewObject<UHACF_TargetingTestSelector>(Config);
			Selection->Behavior.CandidatesToAdd = {MakeActorCandidate(Source), MakeActorCandidate(Excluded), MakeActorCandidate(Included)};
			Config->Selectors = {Selection};
			FHACF_TargetingRequest Request = MakeRequest(Config);
			Request.SourceActor = &Source;
			Request.ExcludedActors.Add(&Excluded);
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();
			Config->bExcludeSourceActor = true;
			const FHACF_TargetingResult ExcludingSelf = Executor->Execute(Request);
			ASSERT_THAT(AreEqual(1, ExcludingSelf.Candidates.Num()));
			ASSERT_THAT(IsTrue(ExcludingSelf.Candidates[0].TargetActor.Get() == &Included));
			Config->bExcludeSourceActor = false;
			const FHACF_TargetingResult KeepingSelf = Executor->Execute(Request);
			ASSERT_THAT(AreEqual(2, KeepingSelf.Candidates.Num()));
			ASSERT_THAT(IsTrue(KeepingSelf.Candidates[0].TargetActor.Get() == &Source));
		}

		TEST_METHOD(Execute_SelectionCleanupPrecedesFiltersAndScorers)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			UHACF_TargetingTestSelector* Selection = NewObject<UHACF_TargetingTestSelector>(Config);
			UHACF_TargetingTestFilter* Filter = NewObject<UHACF_TargetingTestFilter>(Config);
			UHACF_TargetingTestScorer* Scorer = NewObject<UHACF_TargetingTestScorer>(Config);
			Selection->Behavior.CandidatesToAdd = {FHACF_TargetingCandidate(), MakeActorCandidate(Spawner.SpawnActor<AActor>()),
				MakeLocationCandidate(FVector(std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0)), MakeLocationCandidate(FVector(100.0, 0.0, 0.0))};
			Config->Selectors = {Selection};
			Config->Filters = {Filter};
			Config->Scorers = {Scorer};
			const FHACF_TargetingResult Result = NewObject<UHACF_TargetingExecutor>()->Execute(MakeRequest(Config));
			ASSERT_THAT(IsTrue(Result.IsSuccess()));
			ASSERT_THAT(AreEqual(1, Filter->Behavior.ObservedCandidates.Num()));
			ASSERT_THAT(AreEqual(1, Scorer->Behavior.ObservedCandidates.Num()));
			ASSERT_THAT(AreEqual(uint32(3), Filter->Behavior.ObservedCandidates[0].StableSortIndex));
			ASSERT_THAT(AreEqual(uint32(3), Scorer->Behavior.ObservedCandidates[0].StableSortIndex));
		}

		TEST_METHOD(Execute_CleanupRemovingAllCandidatesSkipsRemainingStages)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			UHACF_TargetingTestSelector* Selection = NewObject<UHACF_TargetingTestSelector>(Config);
			UHACF_TargetingTestFilter* Filter = NewObject<UHACF_TargetingTestFilter>(Config);
			UHACF_TargetingTestScorer* Scorer = NewObject<UHACF_TargetingTestScorer>(Config);
			AActor& Excluded = Spawner.SpawnActor<AActor>();
			Selection->Behavior.CandidatesToAdd = {MakeActorCandidate(Excluded)};
			Config->Selectors = {Selection};
			Config->Filters = {Filter};
			Config->Scorers = {Scorer};
			FHACF_TargetingRequest Request = MakeRequest(Config);
			Request.ExcludedActors.Add(&Excluded);
			const FHACF_TargetingResult Result = NewObject<UHACF_TargetingExecutor>()->Execute(Request);
			ASSERT_THAT(IsTrue(Result.Status == EHACF_TargetingResultStatus::NoCandidates));
			ASSERT_THAT(AreEqual(0, Filter->Behavior.ExecuteCount));
			ASSERT_THAT(AreEqual(0, Scorer->Behavior.ExecuteCount));
		}

		TEST_METHOD(Execute_LocationDuplicatesRemainDistinct)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			Config->MaxTargets = 4;
			UHACF_TargetingTestSelector* Selection = NewObject<UHACF_TargetingTestSelector>(Config);
			Selection->Behavior.CandidatesToAdd = {MakeLocationCandidate(FVector(100.0, 0.0, 0.0)),
				MakeLocationCandidate(FVector(100.0, 0.0, 0.0), 0.0f, true, FVector::RightVector)};
			Config->Selectors = {Selection};
			const FHACF_TargetingResult Result = NewObject<UHACF_TargetingExecutor>()->Execute(MakeRequest(Config));
			ASSERT_THAT(AreEqual(2, Result.Candidates.Num()));
			ASSERT_THAT(IsFalse(Result.Candidates[0].bHasDirection));
			ASSERT_THAT(IsTrue(Result.Candidates[1].bHasDirection));
		}

		TEST_METHOD(Execute_FinalCleanupRemovesCandidatesAddedByScorer)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->MaxTargets = 2;
			AActor& Included = Spawner.SpawnActor<AActor>();
			AActor& Excluded = Spawner.SpawnActor<AActor>();
			AActor& Source = Spawner.SpawnActor<AActor>();
			UHACF_TargetingTestSelector* Selection = NewObject<UHACF_TargetingTestSelector>(Config);
			UHACF_TargetingTestScorer* Scorer = NewObject<UHACF_TargetingTestScorer>(Config);
			Selection->Behavior.CandidatesToAdd = {MakeActorCandidate(Included, 1.0f)};
			Scorer->Behavior.CandidatesToAdd = {MakeActorCandidate(Included, 100.0f), MakeActorCandidate(Excluded, 100.0f),
				MakeActorCandidate(Source, 100.0f), FHACF_TargetingCandidate(), MakeLocationCandidate(FVector(100.0, 0.0, 0.0), 100.0f)};
			Config->Selectors = {Selection};
			Config->Scorers = {Scorer};
			Config->bExcludeSourceActor = true;
			FHACF_TargetingRequest Request = MakeRequest(Config);
			Request.SourceActor = &Source;
			Request.ExcludedActors.Add(&Excluded);
			const FHACF_TargetingResult Result = NewObject<UHACF_TargetingExecutor>()->Execute(Request);
			ASSERT_THAT(AreEqual(1, Result.Candidates.Num()));
			ASSERT_THAT(IsTrue(Result.Candidates[0].TargetActor.Get() == &Included));
			ASSERT_THAT(IsTrue(FMath::IsNearlyEqual(1.0f, Result.Candidates[0].Score)));
		}

		TEST_METHOD(ForwardSector_SourceExclusionRespectsConfigWhenCalledDirectly)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			AActor& Source = Spawner.SpawnActor<AActor>();
			UHACF_TargetFilter_ActorForwardSectorAndRange* Filter = NewObject<UHACF_TargetFilter_ActorForwardSectorAndRange>(Config);
			FHACF_TargetingRequest Request = MakeRequest(Config);
			Request.SourceActor = &Source;
			Request.Origin = Source.GetActorLocation();
			FHACF_TargetingExecutionContext Context;
			ASSERT_THAT(IsTrue(Context.Initialize(Request)));
			Config->bExcludeSourceActor = false;
			Context.Candidates = {MakeActorCandidate(Source)};
			ASSERT_THAT(IsTrue(Filter->Filter(Context) == EHACF_TargetingProcessResult::Success));
			ASSERT_THAT(AreEqual(1, Context.Candidates.Num()));
			Config->bExcludeSourceActor = true;
			Context.Candidates = {MakeActorCandidate(Source)};
			ASSERT_THAT(IsTrue(Filter->Filter(Context) == EHACF_TargetingProcessResult::Success));
			ASSERT_THAT(IsTrue(Context.Candidates.IsEmpty()));
		}

		TEST_METHOD(Execute_ValidRequestWithoutProcessors_ReturnsNoCandidates)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();
			const FHACF_TargetingResult Result = Executor->Execute(MakeRequest(Config));

			ASSERT_THAT(IsTrue(Result.Status == EHACF_TargetingResultStatus::NoCandidates));
			ASSERT_THAT(IsTrue(Result.Candidates.IsEmpty()));
			ASSERT_THAT(IsFalse(Result.bHasTargetLocation));
			ASSERT_THAT(IsFalse(Result.bHasTargetDirection));
			ASSERT_THAT(IsTrue(Result.GetPrimaryCandidate() == nullptr));
		}

		TEST_METHOD(Execute_MissingConfig_ReturnsInvalidRequest)
		{
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();
			const FHACF_TargetingResult Result = Executor->Execute(FHACF_TargetingRequest());

			ASSERT_THAT(IsTrue(Result.Status == EHACF_TargetingResultStatus::InvalidRequest));
			ASSERT_THAT(IsTrue(Result.Candidates.IsEmpty()));
		}

		TEST_METHOD(Execute_InvalidEffectiveParameters_ReturnsInvalidRequest)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();
			FHACF_TargetingRequest Request = MakeRequest(Config);

			// 에디터 Clamp를 우회한 런타임 입력도 안전하게 거부해야 한다.
			Config->DefaultRange = 0.0f;
			ASSERT_THAT(IsTrue(Executor->Execute(Request).Status == EHACF_TargetingResultStatus::InvalidRequest));

			Config->DefaultRange = 500.0f;
			Config->MaxTargets = 0;
			ASSERT_THAT(IsTrue(Executor->Execute(Request).Status == EHACF_TargetingResultStatus::InvalidRequest));

			Config->MaxTargets = 1;
			Request.bOverrideRange = true;
			Request.RangeOverride = std::numeric_limits<float>::quiet_NaN();
			ASSERT_THAT(IsTrue(Executor->Execute(Request).Status == EHACF_TargetingResultStatus::InvalidRequest));

			Request.bOverrideRange = false;
			Request.bOverrideMaxTargets = true;
			Request.MaxTargetsOverride = 0;
			ASSERT_THAT(IsTrue(Executor->Execute(Request).Status == EHACF_TargetingResultStatus::InvalidRequest));
		}

		TEST_METHOD(Execute_ProcessorsRunInSelectionFilterScoreOrder)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			Config->MaxTargets = 4;

			TArray<int32> ExecutionOrder;
			UHACF_TargetingTestSelector* Selection0 = NewObject<UHACF_TargetingTestSelector>(Config);
			UHACF_TargetingTestSelector* Selection1 = NewObject<UHACF_TargetingTestSelector>(Config);
			UHACF_TargetingTestFilter* Filter = NewObject<UHACF_TargetingTestFilter>(Config);
			UHACF_TargetingTestScorer* Score = NewObject<UHACF_TargetingTestScorer>(Config);

			Selection0->Behavior.CandidatesToAdd.Add(MakeLocationCandidate(FVector(100.0, 0.0, 0.0)));
			
			Selection0->Behavior.ExecutionToken = 1;
			Selection1->Behavior.ExecutionToken = 2;
			Filter->Behavior.ExecutionToken = 3;
			Score->Behavior.ExecutionToken = 4;
			
			Selection0->Behavior.ExecutionOrder = &ExecutionOrder;
			Selection1->Behavior.ExecutionOrder = &ExecutionOrder;
			Filter->Behavior.ExecutionOrder = &ExecutionOrder;
			Score->Behavior.ExecutionOrder = &ExecutionOrder;

			Config->Selectors = {Selection0, Selection1};
			Config->Filters = {Filter};
			Config->Scorers = {Score};

			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();
			const FHACF_TargetingResult Result = Executor->Execute(MakeRequest(Config));

			ASSERT_THAT(IsTrue(Result.IsSuccess()));
			
			ASSERT_THAT(IsTrue(ExecutionOrder == TArray<int32>({1, 2, 3, 4})));
			
			ASSERT_THAT(IsTrue(Selection0->Behavior.ObservedPhases[0] == EHACF_TargetingPhase::Selection));
			ASSERT_THAT(IsTrue(Selection1->Behavior.ObservedPhases[0] == EHACF_TargetingPhase::Selection));
			ASSERT_THAT(IsTrue(Filter->Behavior.ObservedPhases[0] == EHACF_TargetingPhase::Filter));
			ASSERT_THAT(IsTrue(Score->Behavior.ObservedPhases[0] == EHACF_TargetingPhase::Score));
			
			ASSERT_THAT(AreEqual(0, Selection0->Behavior.ObservedProcessorIndices[0]));
			ASSERT_THAT(AreEqual(1, Selection1->Behavior.ObservedProcessorIndices[0]));
			ASSERT_THAT(AreEqual(0, Filter->Behavior.ObservedProcessorIndices[0]));
			ASSERT_THAT(AreEqual(0, Score->Behavior.ObservedProcessorIndices[0]));
		}

		TEST_METHOD(Execute_NoCandidatesSkipsRemainingStages)
		{
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();

			// Selection 결과가 비어 있으면 Filter와 Score를 실행하지 않는다.
			UHACF_TargetingConfig* EmptySelectionConfig = NewObject<UHACF_TargetingConfig>();
			UHACF_TargetingTestSelector* EmptySelection = NewObject<UHACF_TargetingTestSelector>(EmptySelectionConfig);
			UHACF_TargetingTestFilter* SkippedFilter = NewObject<UHACF_TargetingTestFilter>(EmptySelectionConfig);
			UHACF_TargetingTestScorer* SkippedScore = NewObject<UHACF_TargetingTestScorer>(EmptySelectionConfig);
			EmptySelectionConfig->Selectors = {EmptySelection};
			EmptySelectionConfig->Filters = {SkippedFilter};
			EmptySelectionConfig->Scorers = {SkippedScore};

			const FHACF_TargetingResult EmptySelectionResult = Executor->Execute(MakeRequest(EmptySelectionConfig));
			ASSERT_THAT(IsTrue(EmptySelectionResult.Status == EHACF_TargetingResultStatus::NoCandidates));
			ASSERT_THAT(AreEqual(1, EmptySelection->Behavior.ExecuteCount));
			ASSERT_THAT(AreEqual(0, SkippedFilter->Behavior.ExecuteCount));
			ASSERT_THAT(AreEqual(0, SkippedScore->Behavior.ExecuteCount));

			// Filter가 마지막 후보를 제거한 경우에도 Score는 실행하지 않는다.
			UHACF_TargetingConfig* EmptyFilterConfig = NewObject<UHACF_TargetingConfig>();
			EmptyFilterConfig->TargetCategory = EHACF_TargetCategory::Location;
			
			UHACF_TargetingTestSelector* Selection = NewObject<UHACF_TargetingTestSelector>(EmptyFilterConfig);
			UHACF_TargetingTestFilter* EmptyingFilter = NewObject<UHACF_TargetingTestFilter>(EmptyFilterConfig);
			UHACF_TargetingTestScorer* ScoreAfterFilter = NewObject<UHACF_TargetingTestScorer>(EmptyFilterConfig);
			
			Selection->Behavior.CandidatesToAdd.Add(MakeLocationCandidate(FVector(100.0, 0.0, 0.0)));
			EmptyingFilter->Behavior.bResetCandidates = true;
			EmptyFilterConfig->Selectors = {Selection};
			EmptyFilterConfig->Filters = {EmptyingFilter};
			EmptyFilterConfig->Scorers = {ScoreAfterFilter};

			const FHACF_TargetingResult EmptyFilterResult = Executor->Execute(MakeRequest(EmptyFilterConfig));
			ASSERT_THAT(IsTrue(EmptyFilterResult.Status == EHACF_TargetingResultStatus::NoCandidates));
			ASSERT_THAT(AreEqual(1, EmptyingFilter->Behavior.ExecuteCount));
			ASSERT_THAT(AreEqual(0, ScoreAfterFilter->Behavior.ExecuteCount));
		}

		TEST_METHOD(Execute_ProcessorFailureStopsPipelineAndReturnsFailureStatus)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			
			UHACF_TargetingTestSelector* FailingProcessor = NewObject<UHACF_TargetingTestSelector>(Config);
			UHACF_TargetingTestSelector* ProcessorAfterFailure = NewObject<UHACF_TargetingTestSelector>(Config);
			
			FailingProcessor->Behavior.CandidatesToAdd.Add(MakeLocationCandidate(FVector(100.0, 0.0, 0.0)));
			FailingProcessor->Behavior.bFailContext = true;
			FailingProcessor->Behavior.ReturnResult = EHACF_TargetingProcessResult::Failed;
			Config->Selectors = {FailingProcessor, ProcessorAfterFailure};

			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();
			const FHACF_TargetingResult Result = Executor->Execute(MakeRequest(Config));

			ASSERT_THAT(IsTrue(Result.Status == EHACF_TargetingResultStatus::Failed));
			ASSERT_THAT(IsTrue(Result.Candidates.IsEmpty()));
			ASSERT_THAT(AreEqual(0, ProcessorAfterFailure->Behavior.ExecuteCount));

			// Config에 유효하지 않은 Processor가 들어오면 실행 실패가 아니라 요청 오류로 분류한다.
			UHACF_TargetingConfig* InvalidProcessorConfig = NewObject<UHACF_TargetingConfig>();
			InvalidProcessorConfig->Selectors.Add(nullptr);
			const FHACF_TargetingResult InvalidProcessorResult = Executor->Execute(MakeRequest(InvalidProcessorConfig));
			ASSERT_THAT(IsTrue(InvalidProcessorResult.Status == EHACF_TargetingResultStatus::InvalidRequest));
		}

		TEST_METHOD(Execute_CancelledContextStopsBeforeNextProcessor)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			UHACF_TargetingTestSelector* CancellingProcessor = NewObject<UHACF_TargetingTestSelector>(Config);
			UHACF_TargetingTestSelector* ProcessorAfterCancellation = NewObject<UHACF_TargetingTestSelector>(Config);
			CancellingProcessor->Behavior.CandidatesToAdd.Add(MakeLocationCandidate(FVector(100.0, 0.0, 0.0)));
			CancellingProcessor->Behavior.bCancelContext = true;
			Config->Selectors = {CancellingProcessor, ProcessorAfterCancellation};

			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();
			const FHACF_TargetingResult Result = Executor->Execute(MakeRequest(Config));

			ASSERT_THAT(IsTrue(Result.Status == EHACF_TargetingResultStatus::Cancelled));
			ASSERT_THAT(IsTrue(Result.Candidates.IsEmpty()));
			ASSERT_THAT(AreEqual(0, ProcessorAfterCancellation->Behavior.ExecuteCount));
		}

		TEST_METHOD(Execute_ResultFiltersInvalidAndWrongCategoryCandidates)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			Config->MaxTargets = 10;
			UHACF_TargetingTestSelector* Selection = NewObject<UHACF_TargetingTestSelector>(Config);

			const FVector ValidLocation(100.0, 0.0, 0.0);
			Selection->Behavior.CandidatesToAdd.Add(MakeLocationCandidate(ValidLocation, 10.0f));

			FHACF_TargetingCandidate WrongCategory;
			WrongCategory.TargetCategory = EHACF_TargetCategory::Actor;
			WrongCategory.TargetActor = &Spawner.SpawnActor<AActor>();
			WrongCategory.TargetLocation = FVector(200.0, 0.0, 0.0);
			Selection->Behavior.CandidatesToAdd.Add(WrongCategory);

			Selection->Behavior.CandidatesToAdd.Add(MakeLocationCandidate(
				FVector(std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0)));
			Selection->Behavior.CandidatesToAdd.Add(MakeLocationCandidate(
				FVector(300.0, 0.0, 0.0), std::numeric_limits<float>::infinity()));
			Selection->Behavior.CandidatesToAdd.Add(MakeLocationCandidate(
				FVector(400.0, 0.0, 0.0), 0.0f, true, FVector::ZeroVector));
			Config->Selectors = {Selection};

			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();
			const FHACF_TargetingResult Result = Executor->Execute(MakeRequest(Config));

			ASSERT_THAT(IsTrue(Result.IsSuccess()));
			ASSERT_THAT(AreEqual(1, Result.Candidates.Num()));
			ASSERT_THAT(IsTrue(Result.Candidates[0].TargetLocation.Equals(ValidLocation)));
		}

		TEST_METHOD(Execute_SortsCandidatesDeterministicallyAndAppliesMaxTargets)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			Config->MaxTargets = 2;
			UHACF_TargetingTestSelector* Selection = NewObject<UHACF_TargetingTestSelector>(Config);
			Selection->Behavior.CandidatesToAdd = {
				MakeLocationCandidate(FVector(100.0, 0.0, 0.0), 5.0f),
				MakeLocationCandidate(FVector(200.0, 0.0, 0.0), 10.0f),
				MakeLocationCandidate(FVector(300.0, 0.0, 0.0), 10.0f)
			};
			Config->Selectors = {Selection};
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();

			Config->SortMode = EHACF_TargetSortMode::ScoreDescending;
			const FHACF_TargetingResult Descending = Executor->Execute(MakeRequest(Config));
			ASSERT_THAT(AreEqual(2, Descending.Candidates.Num()));
			ASSERT_THAT(IsTrue(Descending.Candidates[0].TargetLocation.Equals(FVector(200.0, 0.0, 0.0))));
			ASSERT_THAT(IsTrue(Descending.Candidates[1].TargetLocation.Equals(FVector(300.0, 0.0, 0.0))));
			ASSERT_THAT(IsTrue(Descending.Candidates[0].StableSortIndex < Descending.Candidates[1].StableSortIndex));

			Config->SortMode = EHACF_TargetSortMode::ScoreAscending;
			Config->MaxTargets = 3;
			const FHACF_TargetingResult Ascending = Executor->Execute(MakeRequest(Config));
			ASSERT_THAT(AreEqual(3, Ascending.Candidates.Num()));
			ASSERT_THAT(IsTrue(Ascending.Candidates[0].TargetLocation.Equals(FVector(100.0, 0.0, 0.0))));
			ASSERT_THAT(IsTrue(Ascending.Candidates[1].TargetLocation.Equals(FVector(200.0, 0.0, 0.0))));
			ASSERT_THAT(IsTrue(Ascending.Candidates[2].TargetLocation.Equals(FVector(300.0, 0.0, 0.0))));

			FHACF_TargetingRequest OverrideRequest = MakeRequest(Config);
			OverrideRequest.bOverrideMaxTargets = true;
			OverrideRequest.MaxTargetsOverride = 1;
			const FHACF_TargetingResult Overridden = Executor->Execute(OverrideRequest);
			ASSERT_THAT(AreEqual(1, Overridden.Candidates.Num()));
		}

		TEST_METHOD(Execute_PrimaryCandidatePopulatesConvenienceResult)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			UHACF_TargetingTestSelector* Selection = NewObject<UHACF_TargetingTestSelector>(Config);
			const FVector TargetLocation(10.0, 20.0, 30.0);
			const FVector TargetDirection(0.0, 1.0, 0.0);
			Selection->Behavior.CandidatesToAdd.Add(MakeLocationCandidate(TargetLocation, 1.0f, true, TargetDirection));
			Config->Selectors = {Selection};

			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();
			const FHACF_TargetingResult DirectedResult = Executor->Execute(MakeRequest(Config));

			ASSERT_THAT(IsTrue(DirectedResult.IsSuccess()));
			ASSERT_THAT(IsNotNull(DirectedResult.GetPrimaryCandidate()));
			ASSERT_THAT(IsTrue(DirectedResult.bHasTargetLocation));
			ASSERT_THAT(IsTrue(DirectedResult.TargetLocation.Equals(TargetLocation)));
			ASSERT_THAT(IsTrue(DirectedResult.bHasTargetDirection));
			ASSERT_THAT(IsTrue(DirectedResult.TargetDirection.Equals(TargetDirection)));

			// 방향이 없는 후보는 편의 결과에도 방향 없음과 0 벡터를 명시한다.
			Selection->Behavior.CandidatesToAdd = {MakeLocationCandidate(TargetLocation)};
			const FHACF_TargetingResult UndirectedResult = Executor->Execute(MakeRequest(Config));
			ASSERT_THAT(IsFalse(UndirectedResult.bHasTargetDirection));
			ASSERT_THAT(IsTrue(UndirectedResult.TargetDirection.IsZero()));
		}

		TEST_METHOD(Execute_SequentialRequestsDoNotLeakExecutionContext)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();
			Config->TargetCategory = EHACF_TargetCategory::Location;
			Config->MaxTargets = 4;
			UHACF_TargetingTestSelector* Selection = NewObject<UHACF_TargetingTestSelector>(Config);
			Config->Selectors = {Selection};
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();

			Selection->Behavior.CandidatesToAdd = {MakeLocationCandidate(FVector(100.0, 0.0, 0.0))};
			Selection->Behavior.bFailContext = true;
			Selection->Behavior.ReturnResult = EHACF_TargetingProcessResult::Failed;
			ASSERT_THAT(IsTrue(Executor->Execute(MakeRequest(Config)).Status == EHACF_TargetingResultStatus::Failed));

			Selection->Behavior.bFailContext = false;
			Selection->Behavior.ReturnResult = EHACF_TargetingProcessResult::Success;
			Selection->Behavior.CandidatesToAdd = {
				MakeLocationCandidate(FVector(200.0, 0.0, 0.0)),
				MakeLocationCandidate(FVector(300.0, 0.0, 0.0))
			};
			const FHACF_TargetingResult SecondResult = Executor->Execute(MakeRequest(Config));
			ASSERT_THAT(IsTrue(SecondResult.IsSuccess()));
			ASSERT_THAT(AreEqual(2, SecondResult.Candidates.Num()));
			ASSERT_THAT(AreEqual(static_cast<uint32>(0), SecondResult.Candidates[0].StableSortIndex));

			Selection->Behavior.CandidatesToAdd = {MakeLocationCandidate(FVector(400.0, 0.0, 0.0))};
			const FHACF_TargetingResult ThirdResult = Executor->Execute(MakeRequest(Config));
			ASSERT_THAT(IsTrue(ThirdResult.IsSuccess()));
			ASSERT_THAT(AreEqual(1, ThirdResult.Candidates.Num()));
			ASSERT_THAT(IsTrue(ThirdResult.Candidates[0].TargetLocation.Equals(FVector(400.0, 0.0, 0.0))));
			ASSERT_THAT(AreEqual(static_cast<uint32>(0), ThirdResult.Candidates[0].StableSortIndex));
		}
	};
}
