#include "CQTest.h"
#include "Targeting/HACF_TargetingConfig.h"
#include "Targeting/HACF_TargetingExecutor.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(FHACF_TargetingSystemTest, "HyperActionCombat.Tests.Targeting")
	{
		TEST_METHOD(Execute_ValidRequestWithoutTasks_ReturnsNoCandidates)
		{
			UHACF_TargetingConfig* Config = NewObject<UHACF_TargetingConfig>();

			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();

			FHACF_TargetingRequest Request;
			Request.Config = Config;

			const FHACF_TargetingResult Result = Executor->Execute(Request);

			ASSERT_THAT(IsTrue(Result.Status == EHACF_TargetingResultStatus::NoCandidates));

			ASSERT_THAT(IsTrue(Result.Candidates.IsEmpty()));
			ASSERT_THAT(IsFalse(Result.bHasTargetLocation));
			ASSERT_THAT(IsFalse(Result.bHasTargetDirection));
			ASSERT_THAT(IsTrue(Result.GetPrimaryCandidate() == nullptr));
		}
		
		TEST_METHOD(Execute_MissingConfig_ReturnsInvalidRequest)
		{
			UHACF_TargetingExecutor* Executor = NewObject<UHACF_TargetingExecutor>();

			const FHACF_TargetingRequest Request;
			const FHACF_TargetingResult Result = Executor->Execute(Request);

			ASSERT_THAT(IsTrue(Result.Status == EHACF_TargetingResultStatus::InvalidRequest));
			ASSERT_THAT(IsTrue(Result.Candidates.IsEmpty()));
		}
	};
}
