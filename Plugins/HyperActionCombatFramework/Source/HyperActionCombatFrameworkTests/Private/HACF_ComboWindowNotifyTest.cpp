#include "CQTest.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/HACF_CombatGameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifyQueue.h"
#include "Animation/AnimSequence.h"
#include "Animation/HACF_AnimNotifyState_ComboWindow.h"
#include "Abilities/TestCombatGameplayAbility.h"
#include "Components/ActorTestSpawner.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/HACF_CombatComponent.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/UnrealType.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(HACF_ComboWindowRouteTest, "HyperActionCombat.Tests.Combo.WindowRoute")
	{
		UHACF_CombatComponent* Combat = nullptr;
		FGameplayTag WindowTag;
		FHACF_ComboWindowKey NotifyKey;

		BEFORE_EACH()
		{
			Combat = NewObject<UHACF_CombatComponent>();
			ASSERT_THAT(IsNotNull(Combat));

			WindowTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));
			NotifyKey = FHACF_ComboWindowKey::FromQueuedNotify(10);
			ASSERT_THAT(IsTrue(WindowTag.IsValid()));
			ASSERT_THAT(IsTrue(NotifyKey.IsValid()));
		}

		TEST_METHOD(StaleRouteHandle_DoesNotClearReplacementRoute)
		{
			int32 OldBeginCalls = 0;
			int32 OldEndCalls = 0;
			int32 NewBeginCalls = 0;
			int32 NewEndCalls = 0;

			const FHACF_ComboWindowRouteHandle OldHandle = Combat->BindComboWindowRoute(
				FHACF_ComboWindowBeginHandler::CreateLambda(
					[&OldBeginCalls](const FHACF_ComboWindowKey&, const FGameplayTag&)
					{
						++OldBeginCalls;
					}),
				FHACF_ComboWindowEndHandler::CreateLambda(
					[&OldEndCalls](const FHACF_ComboWindowKey&)
					{
						++OldEndCalls;
					}));

			const FHACF_ComboWindowRouteHandle NewHandle = Combat->BindComboWindowRoute(
				FHACF_ComboWindowBeginHandler::CreateLambda(
					[&NewBeginCalls](const FHACF_ComboWindowKey&, const FGameplayTag&)
					{
						++NewBeginCalls;
					}),
				FHACF_ComboWindowEndHandler::CreateLambda(
					[&NewEndCalls](const FHACF_ComboWindowKey&)
					{
						++NewEndCalls;
					}));

			ASSERT_THAT(IsTrue(OldHandle.IsValid()));
			ASSERT_THAT(IsTrue(NewHandle.IsValid()));
			ASSERT_THAT(IsFalse(Combat->UnbindComboWindowRoute(OldHandle)));

			Combat->RouteComboWindowBegin(NotifyKey, WindowTag);
			Combat->RouteComboWindowEnd(NotifyKey);

			ASSERT_THAT(AreEqual(0, OldBeginCalls));
			ASSERT_THAT(AreEqual(0, OldEndCalls));
			ASSERT_THAT(AreEqual(1, NewBeginCalls));
			ASSERT_THAT(AreEqual(1, NewEndCalls));
		}

		TEST_METHOD(UnboundRoute_IgnoresBeginAndEnd)
		{
			int32 BeginCalls = 0;
			int32 EndCalls = 0;

			const FHACF_ComboWindowRouteHandle Handle = Combat->BindComboWindowRoute(
				FHACF_ComboWindowBeginHandler::CreateLambda(
					[&BeginCalls](const FHACF_ComboWindowKey&, const FGameplayTag&)
					{
						++BeginCalls;
					}),
				FHACF_ComboWindowEndHandler::CreateLambda(
					[&EndCalls](const FHACF_ComboWindowKey&)
					{
						++EndCalls;
					}));

			ASSERT_THAT(IsTrue(Combat->UnbindComboWindowRoute(Handle)));

			Combat->RouteComboWindowBegin(NotifyKey, WindowTag);
			Combat->RouteComboWindowEnd(NotifyKey);

			ASSERT_THAT(AreEqual(0, BeginCalls));
			ASSERT_THAT(AreEqual(0, EndCalls));
		}
	};

	TEST_CLASS(HACF_ComboWindowNotifyTest, "HyperActionCombat.Tests.Combo.WindowNotify")
	{
		struct FCombatant
		{
			USkeletalMeshComponent* Mesh = nullptr;
			UAbilitySystemComponent* ASC = nullptr;
			UHACF_CombatComponent* Combat = nullptr;
			UTestCombatGameplayAbility* Ability = nullptr;
		};

		FActorTestSpawner Spawner;
		FCombatant First;
		FCombatant Second;
		UAnimMontage* Montage = nullptr;
		UHACF_AnimNotifyState_ComboWindow* Notify = nullptr;
		FAnimNotifyEvent Event;
		FGameplayTag WindowTag;
		FGameplayTag InputTag;

		void SetUpCombatant(FCombatant& Combatant)
		{
			APawn& Pawn = Spawner.SpawnActor<APawn>();
			Combatant.Mesh = NewObject<USkeletalMeshComponent>(&Pawn);
			Pawn.AddInstanceComponent(Combatant.Mesh);
			
			USkeletalMesh* SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
			ASSERT_THAT(IsNotNull(SkeletalMesh));
			
			Combatant.Mesh->SetSkeletalMeshAsset(SkeletalMesh);
			Combatant.Mesh->SetAnimInstanceClass(UAnimInstance::StaticClass());
			Combatant.Mesh->RegisterComponent();
			ASSERT_THAT(IsNotNull(Combatant.Mesh->GetAnimInstance()));
			
			Pawn.AddInstanceComponent(NewObject<UCharacterMoverComponent>(&Pawn));
			
			Combatant.ASC = NewObject<UAbilitySystemComponent>(&Pawn);
			Pawn.AddInstanceComponent(Combatant.ASC);
			
			Combatant.ASC->RegisterComponent();
			Combatant.ASC->InitAbilityActorInfo(&Pawn, &Pawn);
			Combatant.Combat = NewObject<UHACF_CombatComponent>(&Pawn);
			
			Pawn.AddInstanceComponent(Combatant.Combat);
			Combatant.Combat->RegisterComponent();
			Combatant.Combat->InitializeWithASC(Combatant.ASC, &Pawn);

			const FGameplayAbilitySpecHandle Handle = Combatant.ASC->GiveAbility(FGameplayAbilitySpec(UTestCombatGameplayAbility::StaticClass(), 1));
			ASSERT_THAT(IsTrue(Combatant.ASC->TryActivateAbility(Handle)));
			
			Combatant.Ability = Cast<UTestCombatGameplayAbility>(Combatant.ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
			ASSERT_THAT(IsNotNull(Combatant.Ability));

			UHACF_ComboDefinition* Definition = NewObject<UHACF_ComboDefinition>(Combatant.Ability);
			FHACF_ComboNode& Node = Definition->Nodes.AddDefaulted_GetRef();
			Node.NodeId = TEXT("Attack");
			Node.Montage = Montage;
			Node.Windows.AddDefaulted_GetRef().WindowTag = WindowTag;
			
			FHACF_ComboTransition& Transition = Node.Transitions.AddDefaulted_GetRef();
			Transition.WindowTag = WindowTag;
			Transition.InputTag = InputTag;
			Transition.TargetNodeId = Node.NodeId;
			
			FObjectProperty* DefinitionProperty = FindFProperty<FObjectProperty>(UHACF_CombatGameplayAbility::StaticClass(), TEXT("AttackComboDefinition"));
			ASSERT_THAT(IsNotNull(DefinitionProperty));
			
			DefinitionProperty->SetObjectPropertyValue_InContainer(Combatant.Ability, Definition);
			Combatant.Ability->PlayComboNodeForTest(Node.NodeId);
			
			ASSERT_THAT(IsTrue(Combatant.Ability->IsActive()));
			ASSERT_THAT(IsTrue(Combatant.Mesh->GetAnimInstance()->Montage_IsPlaying(Montage)));
		}

		BEFORE_EACH()
		{
			WindowTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));
			InputTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.Action.Primary"));
			UAnimSequence* Sequence = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));
			ASSERT_THAT(IsNotNull(Sequence));
			
			Montage = UAnimMontage::CreateSlotAnimationAsDynamicMontage(Sequence, TEXT("DefaultSlot"));
			ASSERT_THAT(IsNotNull(Montage));
			ASSERT_THAT(IsFalse(Montage->HasRootMotion()));
			
			Notify = NewObject<UHACF_AnimNotifyState_ComboWindow>();
			Notify->WindowTag = WindowTag;
			Event.NotifyStateClass = Notify;
			SetUpCombatant(First);
		}

		AFTER_EACH()
		{
			if (First.ASC)
			{
				First.ASC->CancelAllAbilities();
			}
			if (Second.ASC)
			{
				Second.ASC->CancelAllAbilities();
			}
		}

		TEST_METHOD(BranchingBegin_WithoutQueuedNotifyInstanceID_OpensOnlyOneWindowForDuplicateBegin)
		{
			FBranchingPointNotifyPayload Payload(First.Mesh, Montage, &Event, 10);
			Notify->BranchingPointNotifyBegin(Payload);
			Notify->BranchingPointNotifyBegin(Payload);
			
			ASSERT_THAT(AreEqual(1, First.Combat->CloseAllActionWindowsFromSource(First.Ability)));
		}

		TEST_METHOD(BranchingEnd_ClosesMatchingWindow)
		{
			FBranchingPointNotifyPayload Payload(First.Mesh, Montage, &Event, 10);
			Notify->BranchingPointNotifyBegin(Payload);
			
			// UE stores a copy in ActiveStateBranchingPoints for the End callback.
			FAnimNotifyEvent EndEvent = Event;
			FBranchingPointNotifyPayload EndPayload(First.Mesh, Montage, &EndEvent, 10, true);
			Notify->BranchingPointNotifyEnd(EndPayload);
			
			ASSERT_THAT(AreEqual(0, First.Combat->CloseAllActionWindowsFromSource(First.Ability)));
		}

		TEST_METHOD(OldMontageEnd_DoesNotCloseNewPlaybackWindow)
		{
			FBranchingPointNotifyPayload OldPayload(First.Mesh, Montage, &Event, 10);
			FBranchingPointNotifyPayload NewPayload(First.Mesh, Montage, &Event, 11);
			
			Notify->BranchingPointNotifyBegin(OldPayload);
			First.Ability->PlayComboNodeForTest(TEXT("Attack"));
			
			Notify->BranchingPointNotifyBegin(NewPayload);
			Notify->BranchingPointNotifyEnd(OldPayload);
			
			ASSERT_THAT(AreEqual(1, First.Combat->CloseAllActionWindowsFromSource(First.Ability)));
		}

		TEST_METHOD(SharedNotify_EndForOneActorLeavesOtherActorWindowOpen)
		{
			SetUpCombatant(Second);
			
			FBranchingPointNotifyPayload FirstPayload(First.Mesh, Montage, &Event, 10);
			FBranchingPointNotifyPayload SecondPayload(Second.Mesh, Montage, &Event, 10);
			
			Notify->BranchingPointNotifyBegin(FirstPayload);
			Notify->BranchingPointNotifyBegin(SecondPayload);
			Notify->BranchingPointNotifyEnd(FirstPayload);
			
			ASSERT_THAT(AreEqual(0, First.Combat->CloseAllActionWindowsFromSource(First.Ability)));
			ASSERT_THAT(AreEqual(1, Second.Combat->CloseAllActionWindowsFromSource(Second.Ability)));
		}

		TEST_METHOD(DifferentNotifyPlacements_HaveIndependentWindows)
		{
			FAnimNotifyEvent OtherEvent = Event;
			UHACF_AnimNotifyState_ComboWindow* OtherNotify = NewObject<UHACF_AnimNotifyState_ComboWindow>();
			OtherNotify->WindowTag = WindowTag;
			OtherEvent.NotifyStateClass = OtherNotify;
			
			FBranchingPointNotifyPayload FirstPayload(First.Mesh, Montage, &Event, 10);
			FBranchingPointNotifyPayload SecondPayload(First.Mesh, Montage, &OtherEvent, 10);
			
			Notify->BranchingPointNotifyBegin(FirstPayload);
			OtherNotify->BranchingPointNotifyBegin(SecondPayload);
			Notify->BranchingPointNotifyEnd(FirstPayload);
			
			ASSERT_THAT(AreEqual(1, First.Combat->CloseAllActionWindowsFromSource(First.Ability)));
		}

		TEST_METHOD(QueuedAndBranchingNotifications_DoNotShareIntegerIdentity)
		{
			FAnimNotifyEventReference Reference;
			Reference.SetNotifyInstanceID(10);
			
			FBranchingPointNotifyPayload Payload(First.Mesh, Montage, &Event, 10);
			Notify->NotifyBegin(First.Mesh, Montage, 1.f, Reference);
			Notify->BranchingPointNotifyBegin(Payload);
			Notify->NotifyEnd(First.Mesh, Montage, Reference);
			
			ASSERT_THAT(AreEqual(1, First.Combat->CloseAllActionWindowsFromSource(First.Ability)));
		}

		TEST_METHOD(BufferedInputAtBranchingBegin_AdvancesComboAndClosesOldWindows)
		{
			const int32 OldMontageID = First.Mesh->GetAnimInstance()->GetActiveInstanceForMontage(Montage)->GetInstanceID();
			First.Combat->RecordBufferedInput(InputTag);
			First.Combat->ProcessAbilityInput(0.1f, false);
			
			FBranchingPointNotifyPayload Payload(First.Mesh, Montage, &Event, OldMontageID);
			Notify->BranchingPointNotifyBegin(Payload);
			
			FAnimMontageInstance* NewInstance = First.Mesh->GetAnimInstance()->GetActiveInstanceForMontage(Montage);
			ASSERT_THAT(IsNotNull(NewInstance));
			
			ASSERT_THAT(IsTrue(NewInstance->GetInstanceID() != OldMontageID));
			ASSERT_THAT(AreEqual(0, First.Combat->CloseAllActionWindowsFromSource(First.Ability)));
			ASSERT_THAT(IsFalse(First.Combat->ProcessBufferedInput()));
		}

		TEST_METHOD(CancelAbility_CleansUpBranchingAndQueuedWindows)
		{
			FBranchingPointNotifyPayload Payload(First.Mesh, Montage, &Event, 10);
			Notify->BranchingPointNotifyBegin(Payload);
			
			First.Combat->RouteComboWindowBegin(FHACF_ComboWindowKey::FromQueuedNotify(20), WindowTag);
			First.ASC->CancelAllAbilities();
			
			ASSERT_THAT(IsFalse(First.Ability->IsActive()));
			ASSERT_THAT(AreEqual(0, First.Combat->CloseAllActionWindowsFromSource(First.Ability)));

			Notify->BranchingPointNotifyBegin(Payload);
			ASSERT_THAT(AreEqual(0, First.Combat->CloseAllActionWindowsFromSource(First.Ability)));

			Notify->BranchingPointNotifyEnd(Payload);
		}

		TEST_METHOD(InactiveAbility_CannotReplaceExistingComboWindowRoute)
		{
			First.ASC->CancelAllAbilities();
			ASSERT_THAT(IsFalse(First.Ability->IsActive()));

			int32 BeginCalls = 0;
			const FHACF_ComboWindowRouteHandle ExistingRoute = First.Combat->BindComboWindowRoute(
				FHACF_ComboWindowBeginHandler::CreateLambda(
					[&BeginCalls](const FHACF_ComboWindowKey&, const FGameplayTag&)
					{
						++BeginCalls;
					}),
				FHACF_ComboWindowEndHandler::CreateLambda(
					[](const FHACF_ComboWindowKey&)
					{
					}));

			ASSERT_THAT(IsTrue(ExistingRoute.IsValid()));

			First.Ability->PlayComboNodeForTest(TEXT("Attack"));
			First.Combat->RouteComboWindowBegin(FHACF_ComboWindowKey::FromQueuedNotify(30), WindowTag);

			ASSERT_THAT(AreEqual(1, BeginCalls));
		}
	};
}
