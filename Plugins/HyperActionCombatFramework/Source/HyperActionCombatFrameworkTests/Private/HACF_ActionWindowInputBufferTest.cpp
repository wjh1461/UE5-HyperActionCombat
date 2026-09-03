#include "CQTest.h"
#include "Components/ActorTestSpawner.h"
#include "Core/HACF_CombatComponent.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(HACF_ActionWindowInputBufferTest, "HyperActionCombat.Tests.InputBuffer.ActionWindow")
	{
		FActorTestSpawner Spawner;
		APawn* DummyPawn = nullptr;
		UHACF_CombatComponent* CombatComponent = nullptr;

		BEFORE_EACH()
		{
			DummyPawn = &Spawner.SpawnActor<APawn>();
			ASSERT_THAT(IsNotNull(DummyPawn));

			CombatComponent = NewObject<UHACF_CombatComponent>(DummyPawn, TEXT("CombatComponent"));
			CombatComponent->RegisterComponent();
			DummyPawn->AddInstanceComponent(CombatComponent);
		}

		TEST_METHOD(OpenWindow_ConsumesMatchingBufferedInputExactlyOnce)
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

			ASSERT_THAT(AreEqual(1, DispatchCount));
		}

		TEST_METHOD(Consume_SkipsIncompatibleHeadAndConsumesOldestCompatibleInput)
		{
			const FGameplayTag IncompatibleInputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Ability.Dodge"));
			const FGameplayTag CompatibleInputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));

			CombatComponent->RecordBufferedInput(IncompatibleInputTag);
			CombatComponent->RecordBufferedInput(CompatibleInputTag);

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

			ASSERT_THAT(AreEqual(1, CompatibleDispatchCount));
			ASSERT_THAT(IsTrue(CompatibleDispatchedInputTag == CompatibleInputTag));

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

			ASSERT_THAT(AreEqual(1, CompatibleDispatchCount));
			ASSERT_THAT(AreEqual(1, IncompatibleDispatchCount));
		}

		TEST_METHOD(HandlerOpensNextWindow_ConsumesPendingInputAfterDispatch)
		{
			const FGameplayTag FirstInputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));
			const FGameplayTag SecondInputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Secondary"));

			CombatComponent->RecordBufferedInput(FirstInputTag);
			CombatComponent->RecordBufferedInput(SecondInputTag);

			FHACF_ActionWindowDefinition FirstWindowDefinition;
			FirstWindowDefinition.AcceptedInputTags.AddTag(FirstInputTag);

			FHACF_ActionWindowDefinition SecondWindowDefinition;
			SecondWindowDefinition.AcceptedInputTags.AddTag(SecondInputTag);

			int32 FirstDispatchCount = 0;
			int32 SecondDispatchCount = 0;
			FGameplayTag FirstDispatchedInputTag;
			FGameplayTag SecondDispatchedInputTag;
			FHACF_ActionWindowHandle SecondWindowHandle;

			CombatComponent->OpenActionWindow(FirstWindowDefinition, DummyPawn,
				FHACF_ActionWindowInputHandler::CreateLambda(
					[this, &SecondWindowDefinition, &FirstDispatchCount, &SecondDispatchCount,
						&FirstDispatchedInputTag, &SecondDispatchedInputTag,
						&SecondWindowHandle](const FHACF_ActionWindowInputContext& Context)
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
						return true;
					}));

			ASSERT_THAT(AreEqual(1, FirstDispatchCount));
			ASSERT_THAT(IsTrue(FirstDispatchedInputTag == FirstInputTag));
			
			ASSERT_THAT(IsTrue(SecondWindowHandle.IsValid()));
			ASSERT_THAT(AreEqual(1, SecondDispatchCount));
			ASSERT_THAT(IsTrue(SecondDispatchedInputTag == SecondInputTag));
		}
	};
}
