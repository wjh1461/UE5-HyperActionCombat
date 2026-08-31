#include "CQTest.h"
#include "Core/CombatInput/HACF_InputBuffer.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(HACF_InputBufferTest, "HyperActionCombat.Tests.InputBuffer")
	{
		TEST_METHOD(NewBuffer_IsEmpty)
		{
			FHACF_InputBuffer Buffer;

			ASSERT_THAT(IsTrue(Buffer.IsEmpty()));
			ASSERT_THAT(AreEqual(0, Buffer.Num()));
			ASSERT_THAT(IsTrue(Buffer.Find() == nullptr));
		}

		TEST_METHOD(Push_AddsInputAndFindReturnsIt)
		{
			FHACF_InputBuffer Buffer;
			
			const FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));

			FHACF_BufferedInput Input;
			Input.InputTag = TestTag;
			Input.InputTime = 1.0f;
			Input.ExpirationTime = 2.0f;

			const uint32 Sequence = Buffer.Push(Input);
			const FHACF_BufferedInput* FoundInput = Buffer.Find();

			ASSERT_THAT(IsFalse(Buffer.IsEmpty()));
			ASSERT_THAT(AreEqual(1, Buffer.Num()));
			ASSERT_THAT(IsNotNull(FoundInput));
			ASSERT_THAT(IsTrue(Sequence != 0));
			ASSERT_THAT(AreEqual(Sequence, FoundInput->Sequence));
			ASSERT_THAT(IsTrue(FoundInput->InputTag == TestTag));
			ASSERT_THAT(IsTrue(FoundInput->InputTime == 1.0f));
			ASSERT_THAT(IsTrue(FoundInput->ExpirationTime == 2.0f));
		}

		TEST_METHOD(Find_ReturnsOldestInputFirst)
		{
			FHACF_InputBuffer Buffer;
			
			FHACF_BufferedInput FirstInput;
			FirstInput.InputTime = 1.0f;
			FirstInput.ExpirationTime = 3.0f;

			FHACF_BufferedInput SecondInput;
			SecondInput.InputTime = 2.0f;
			SecondInput.ExpirationTime = 4.0f;

			const uint32 FirstSequence = Buffer.Push(FirstInput);
			Buffer.Push(SecondInput);

			const FHACF_BufferedInput* FoundInput = Buffer.Find();
			
			ASSERT_THAT(IsNotNull(FoundInput));
			ASSERT_THAT(AreEqual(FirstSequence, FoundInput->Sequence));
			ASSERT_THAT(IsTrue(FoundInput->InputTime == 1.0f));
		}

		TEST_METHOD(Consume_RemovesMatchingSequence)
		{
			FHACF_InputBuffer Buffer;
			
			FHACF_BufferedInput FirstInput;
			FirstInput.InputTime = 1.0f;
			FirstInput.ExpirationTime = 3.0f;

			FHACF_BufferedInput SecondInput;
			SecondInput.InputTime = 2.0f;
			SecondInput.ExpirationTime = 4.0f;

			const uint32 FirstSequence = Buffer.Push(FirstInput);
			const uint32 SecondSequence = Buffer.Push(SecondInput);

			ASSERT_THAT(IsTrue(FirstSequence != SecondSequence));
			ASSERT_THAT(IsTrue(Buffer.Consume(FirstSequence)));
			ASSERT_THAT(AreEqual(1, Buffer.Num()));

			const FHACF_BufferedInput* RemainingInput = Buffer.Find();
			ASSERT_THAT(IsNotNull(RemainingInput));
			ASSERT_THAT(AreEqual(SecondSequence, RemainingInput->Sequence));
		}

		TEST_METHOD(Consume_UnknownSequenceReturnsFalseAndPreservesBuffer)
		{
			FHACF_InputBuffer Buffer;
			
			FHACF_BufferedInput Input;
			Input.ExpirationTime = 1.0f;
			
			const uint32 ExistingSequence = Buffer.Push(Input);

			ASSERT_THAT(IsFalse(Buffer.Consume(ExistingSequence + 1)));
			ASSERT_THAT(AreEqual(1, Buffer.Num()));
			ASSERT_THAT(AreEqual(ExistingSequence, Buffer.Find()->Sequence));
		}

		TEST_METHOD(RemoveExpired_RemovesOnlyExpiredInputs)
		{
			FHACF_InputBuffer Buffer;
			
			FHACF_BufferedInput ExpiredInput;
			ExpiredInput.ExpirationTime = 2.0f;

			FHACF_BufferedInput ActiveInput;
			ActiveInput.ExpirationTime = 4.0f;

			Buffer.Push(ExpiredInput);
			const uint32 ActiveSequence = Buffer.Push(ActiveInput);

			Buffer.RemoveExpired(3.0);

			ASSERT_THAT(AreEqual(1, Buffer.Num()));
			ASSERT_THAT(AreEqual(ActiveSequence, Buffer.Find()->Sequence));
		}

		TEST_METHOD(RemoveExpired_AtExpirationBoundaryRemovesInput)
		{
			FHACF_InputBuffer Buffer;
			
			FHACF_BufferedInput Input;
			Input.ExpirationTime = 2.0f;
			
			Buffer.Push(Input);

			Buffer.RemoveExpired(2.0);

			ASSERT_THAT(IsTrue(Buffer.IsEmpty()));
			ASSERT_THAT(IsTrue(Buffer.Find() == nullptr));
		}

		TEST_METHOD(Clear_RemovesAllInputs)
		{
			FHACF_InputBuffer Buffer;
			
			FHACF_BufferedInput Input;
			Input.ExpirationTime = 1.0f;
			
			Buffer.Push(Input);
			Buffer.Push(Input);

			Buffer.Clear();

			ASSERT_THAT(IsTrue(Buffer.IsEmpty()));
			ASSERT_THAT(AreEqual(0, Buffer.Num()));
			ASSERT_THAT(IsTrue(Buffer.Find() == nullptr));
		}
	};
}
