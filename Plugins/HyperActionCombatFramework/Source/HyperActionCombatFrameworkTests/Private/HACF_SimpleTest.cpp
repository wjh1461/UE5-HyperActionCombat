#include "CQTest.h"

namespace HyperActionCombatFrameworkTests
{
	 // CQTest 매크로를 사용하여 가장 기본적인 테스트 클래스를 선언합니다.
	// 첫 번째 인자: 테스트 클래스 이름
	// 두 번째 인자: 언리얼 에디터의 Session Frontend(Test 창)에 표시될 카테고리 경로
	TEST_CLASS(HACF_SimpleTest, "HyperActionCombat.Tests.Sanity")
	{
		// 개별 단위 테스트 메서드를 선언합니다.
		TEST_METHOD(SanityCheck_MathWorks)
		{
			// 테스트할 대상과 예상 결과를 정의합니다.
			int32 ExpectedValue = 4;
			int32 ActualValue = 2 + 2;

			// ASSERT_THAT 매크로를 통해 두 값이 일치하는지 단언(Assert)합니다.
			// 일치하지 않으면 테스트가 실패(Fail) 처리됩니다.
			ASSERT_THAT(AreEqual(ExpectedValue, ActualValue));
		}

		TEST_METHOD(SanityCheck_StringCompare)
		{
			FString ExpectedString = TEXT("CQTestWorks");
			FString ActualString = TEXT("CQTest") + FString(TEXT("Works"));

			// 문자열 비교 (내부적으로 Equals 등을 사용해 정확히 비교해 줍니다)
			ASSERT_THAT(AreEqual(ExpectedString, ActualString));
		}
	};
}
