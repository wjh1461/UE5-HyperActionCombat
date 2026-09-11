#include "CQTest.h"
#include "AbilitySystemComponent.h"
#include "Components/ActorTestSpawner.h"
#include "Core/HACF_CombatComponent.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(HACF_ActionWindowInputBufferTest, "HyperActionCombat.Tests.InputBuffer.ActionWindow")
	{
		FActorTestSpawner Spawner;
		APawn* DummyPawn = nullptr;
		UHACF_CombatComponent* CombatComponent = nullptr;
		UAbilitySystemComponent* ASC = nullptr;

		BEFORE_EACH()
		{
			DummyPawn = &Spawner.SpawnActor<APawn>();
			ASSERT_THAT(IsNotNull(DummyPawn));

			ASC = NewObject<UAbilitySystemComponent>(DummyPawn, TEXT("AbilitySystemComponent"));
			ASC->RegisterComponent();
			DummyPawn->AddInstanceComponent(ASC);

			CombatComponent = NewObject<UHACF_CombatComponent>(DummyPawn, TEXT("CombatComponent"));
			CombatComponent->RegisterComponent();
			DummyPawn->AddInstanceComponent(CombatComponent);
			CombatComponent->InitializeWithASC(ASC, DummyPawn);
		}

		TEST_METHOD(ProcessBufferedInput_ConsumesMatchingBufferedInputExactlyOnce)
		{
			const FGameplayTag InputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));
			const FGameplayTag WindowTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));

			CombatComponent->RecordBufferedInput(InputTag);

			FHACF_ActionWindowDefinition Definition;
			Definition.WindowTag = WindowTag;
			Definition.AcceptedInputTags.AddTag(InputTag);

			int32 DispatchCount = 0;
			FHACF_ActionWindowInputContext ReceivedContext;
			const FHACF_ActionWindowHandle Handle = CombatComponent->OpenActionWindow(Definition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[&DispatchCount, &ReceivedContext](const FHACF_ActionWindowInputContext& Context)
					{
						++DispatchCount;
						ReceivedContext = Context;
						return true;
					}));

			ASSERT_THAT(IsTrue(Handle.IsValid()));
			ASSERT_THAT(AreEqual(0, DispatchCount));

			ASSERT_THAT(IsTrue(CombatComponent->ProcessBufferedInput()));
			ASSERT_THAT(AreEqual(1, DispatchCount));
			
			ASSERT_THAT(IsTrue(ReceivedContext.WindowHandle == Handle));
			ASSERT_THAT(IsTrue(ReceivedContext.WindowTag == WindowTag));
			ASSERT_THAT(IsTrue(ReceivedContext.InputTag == InputTag));

			ASSERT_THAT(IsTrue(CombatComponent->CloseActionWindow(Handle)));
			
			CombatComponent->OpenActionWindow(Definition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[&DispatchCount](const FHACF_ActionWindowInputContext&)
					{
						++DispatchCount;
						return true;
					}));

			ASSERT_THAT(IsFalse(CombatComponent->ProcessBufferedInput()));
			ASSERT_THAT(AreEqual(1, DispatchCount));
		}

		TEST_METHOD(ProcessBufferedInput_IncompatibleHeadBlocksLaterCompatibleInput)
		{
			const FGameplayTag IncompatibleInputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Ability.Dodge"));
			const FGameplayTag CompatibleInputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));

			CombatComponent->RecordBufferedInput(IncompatibleInputTag);
			CombatComponent->RecordBufferedInput(CompatibleInputTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);

			FHACF_ActionWindowDefinition CompatibleWindowDefinition;
			CompatibleWindowDefinition.AcceptedInputTags.AddTag(CompatibleInputTag);

			int32 CompatibleDispatchCount = 0;
			FGameplayTag CompatibleDispatchedInputTag;
			CombatComponent->OpenActionWindow(CompatibleWindowDefinition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[&CompatibleDispatchCount, &CompatibleDispatchedInputTag](const FHACF_ActionWindowInputContext& Context)
					{
						++CompatibleDispatchCount;
						CompatibleDispatchedInputTag = Context.InputTag;
						return true;
					}));

			ASSERT_THAT(IsFalse(CombatComponent->ProcessBufferedInput()));
			ASSERT_THAT(AreEqual(0, CompatibleDispatchCount));

			FHACF_ActionWindowDefinition IncompatibleWindowDefinition;
			IncompatibleWindowDefinition.AcceptedInputTags.AddTag(IncompatibleInputTag);

			int32 IncompatibleDispatchCount = 0;
			CombatComponent->OpenActionWindow(IncompatibleWindowDefinition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[&IncompatibleDispatchCount](const FHACF_ActionWindowInputContext&)
					{
						++IncompatibleDispatchCount;
						return true;
					}));

			ASSERT_THAT(IsTrue(CombatComponent->ProcessBufferedInput()));
			ASSERT_THAT(AreEqual(0, CompatibleDispatchCount));
			ASSERT_THAT(AreEqual(1, IncompatibleDispatchCount));

			ASSERT_THAT(IsTrue(CombatComponent->ProcessBufferedInput()));
			ASSERT_THAT(AreEqual(1, CompatibleDispatchCount));
			ASSERT_THAT(IsTrue(CompatibleDispatchedInputTag == CompatibleInputTag));
			ASSERT_THAT(AreEqual(1, IncompatibleDispatchCount));
		}

		TEST_METHOD(HandlerOpensNextWindow_ReentrantProcessDefersPendingInputUntilLaterCall)
		{
			const FGameplayTag FirstInputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));
			const FGameplayTag SecondInputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Secondary"));

			CombatComponent->RecordBufferedInput(FirstInputTag);
			CombatComponent->RecordBufferedInput(SecondInputTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);

			FHACF_ActionWindowDefinition FirstWindowDefinition;
			FirstWindowDefinition.AcceptedInputTags.AddTag(FirstInputTag);

			FHACF_ActionWindowDefinition SecondWindowDefinition;
			SecondWindowDefinition.AcceptedInputTags.AddTag(SecondInputTag);

			int32 FirstDispatchCount = 0;
			int32 SecondDispatchCount = 0;
			FGameplayTag FirstDispatchedInputTag;
			FGameplayTag SecondDispatchedInputTag;
			FHACF_ActionWindowHandle SecondWindowHandle;
			bool bReentrantProcessSucceeded = true;

			CombatComponent->OpenActionWindow(FirstWindowDefinition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[this, &SecondWindowDefinition, &FirstDispatchCount, &SecondDispatchCount,
						&FirstDispatchedInputTag, &SecondDispatchedInputTag,
						&SecondWindowHandle, &bReentrantProcessSucceeded](const FHACF_ActionWindowInputContext& Context)
					{
						++FirstDispatchCount;
						FirstDispatchedInputTag = Context.InputTag;
						SecondWindowHandle = CombatComponent->OpenActionWindow(SecondWindowDefinition, DummyPawn,
							FHACF_ActionWindowInputHandler::CreateLambda(
								[&SecondDispatchCount, &SecondDispatchedInputTag](const FHACF_ActionWindowInputContext& SecondContext)
								{
									++SecondDispatchCount;
									SecondDispatchedInputTag = SecondContext.InputTag;
									return true;
								}));
						bReentrantProcessSucceeded = CombatComponent->ProcessBufferedInput();
						return true;
					}));

			ASSERT_THAT(IsTrue(CombatComponent->ProcessBufferedInput()));
			ASSERT_THAT(AreEqual(1, FirstDispatchCount));
			ASSERT_THAT(IsTrue(FirstDispatchedInputTag == FirstInputTag));
			
			ASSERT_THAT(IsTrue(SecondWindowHandle.IsValid()));
			ASSERT_THAT(IsFalse(bReentrantProcessSucceeded));
			ASSERT_THAT(AreEqual(0, SecondDispatchCount));

			ASSERT_THAT(IsTrue(CombatComponent->ProcessBufferedInput()));
			ASSERT_THAT(AreEqual(1, SecondDispatchCount));
			ASSERT_THAT(IsTrue(SecondDispatchedInputTag == SecondInputTag));
		}

		TEST_METHOD(CloseAllActionWindowsFromSource_ClosesOnlyMatchingWindows)
		{
			const FGameplayTag InputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));
			APawn* SecondSource = &Spawner.SpawnActor<APawn>();
			ASSERT_THAT(IsNotNull(SecondSource));

			FHACF_ActionWindowDefinition Definition;
			Definition.AcceptedInputTags.AddTag(InputTag);

			const FHACF_ActionWindowHandle FirstMatchingHandle = CombatComponent->OpenActionWindow(
				Definition, DummyPawn, FHACF_ActionWindowInputHandler());
			const FHACF_ActionWindowHandle SecondMatchingHandle = CombatComponent->OpenActionWindow(
				Definition, DummyPawn, FHACF_ActionWindowInputHandler());
			const FHACF_ActionWindowHandle OtherSourceHandle = CombatComponent->OpenActionWindow(
				Definition, SecondSource, FHACF_ActionWindowInputHandler());
			const FHACF_ActionWindowHandle SourceLessHandle = CombatComponent->OpenActionWindow(
				Definition, nullptr, FHACF_ActionWindowInputHandler());

			ASSERT_THAT(AreEqual(2, CombatComponent->CloseAllActionWindowsFromSource(DummyPawn)));
			ASSERT_THAT(IsFalse(CombatComponent->IsActionWindowActive(FirstMatchingHandle)));
			ASSERT_THAT(IsFalse(CombatComponent->IsActionWindowActive(SecondMatchingHandle)));
			ASSERT_THAT(IsTrue(CombatComponent->IsActionWindowActive(OtherSourceHandle)));
			ASSERT_THAT(IsTrue(CombatComponent->IsActionWindowActive(SourceLessHandle)));
		}

		TEST_METHOD(ProcessBufferedInput_SelectsHighestPriorityWindow)
		{
			const FGameplayTag InputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));

			FHACF_ActionWindowDefinition LowPriorityDefinition;
			LowPriorityDefinition.AcceptedInputTags.AddTag(InputTag);

			FHACF_ActionWindowDefinition HighPriorityDefinition = LowPriorityDefinition;
			HighPriorityDefinition.Priority = 10;

			int32 LowPriorityDispatchCount = 0;
			int32 HighPriorityDispatchCount = 0;
			CombatComponent->RecordBufferedInput(InputTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);

			CombatComponent->OpenActionWindow(LowPriorityDefinition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[&LowPriorityDispatchCount](const FHACF_ActionWindowInputContext&)
					{
						++LowPriorityDispatchCount;
						return true;
					}));
			CombatComponent->OpenActionWindow(HighPriorityDefinition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[&HighPriorityDispatchCount](const FHACF_ActionWindowInputContext&)
					{
						++HighPriorityDispatchCount;
						return true;
					}));

			ASSERT_THAT(IsTrue(CombatComponent->ProcessBufferedInput()));
			ASSERT_THAT(AreEqual(0, LowPriorityDispatchCount));
			ASSERT_THAT(AreEqual(1, HighPriorityDispatchCount));
		}

		TEST_METHOD(ProcessBufferedInput_SamePrioritySelectsFirstOpenedWindow)
		{
			const FGameplayTag InputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));

			FHACF_ActionWindowDefinition Definition;
			Definition.AcceptedInputTags.AddTag(InputTag);

			int32 FirstDispatchCount = 0;
			int32 SecondDispatchCount = 0;
			CombatComponent->RecordBufferedInput(InputTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);

			const FHACF_ActionWindowHandle FirstHandle = CombatComponent->OpenActionWindow(Definition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[&FirstDispatchCount](const FHACF_ActionWindowInputContext&)
					{
						++FirstDispatchCount;
						return true;
					}));
			CombatComponent->OpenActionWindow(Definition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[&SecondDispatchCount](const FHACF_ActionWindowInputContext&)
					{
						++SecondDispatchCount;
						return true;
					}));

			ASSERT_THAT(IsTrue(CombatComponent->ProcessBufferedInput()));
			ASSERT_THAT(AreEqual(1, FirstDispatchCount));
			ASSERT_THAT(AreEqual(0, SecondDispatchCount));

			ASSERT_THAT(IsTrue(CombatComponent->CloseActionWindow(FirstHandle)));
			CombatComponent->RecordBufferedInput(InputTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);
			ASSERT_THAT(AreEqual(1, SecondDispatchCount));
		}

		TEST_METHOD(HandlerClosesItsWindow_DispatchRemainsSafeAndConsumesInput)
		{
			const FGameplayTag InputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));

			FHACF_ActionWindowDefinition Definition;
			Definition.AcceptedInputTags.AddTag(InputTag);

			int32 DispatchCount = 0;
			bool bCloseSucceeded = false;
			FHACF_ActionWindowHandle Handle;

			CombatComponent->RecordBufferedInput(InputTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);

			Handle = CombatComponent->OpenActionWindow(Definition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[this, &DispatchCount, &bCloseSucceeded, &Handle](const FHACF_ActionWindowInputContext&)
					{
						++DispatchCount;
						bCloseSucceeded = CombatComponent->CloseActionWindow(Handle);
						return true;
					}));

			ASSERT_THAT(IsTrue(CombatComponent->ProcessBufferedInput()));
			ASSERT_THAT(AreEqual(1, DispatchCount));
			ASSERT_THAT(IsTrue(bCloseSucceeded));
			ASSERT_THAT(IsFalse(CombatComponent->IsActionWindowActive(Handle)));
			ASSERT_THAT(IsFalse(CombatComponent->ProcessBufferedInput()));
		}
	};
}
