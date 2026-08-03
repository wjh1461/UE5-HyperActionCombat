#include "AbilitySystemComponent.h"
#include "CQTest.h"
#include "GameplayTagsManager.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbility.h"
#include "Components/ActorTestSpawner.h"
#include "Components/MapTestSpawner.h"
#include "Abilities/TestGameplayAbility.h"
#include "Core/HACF_CombatComponent.h"

namespace HyperActionCombatFrameworkTests
{
	// 1. 테스트 전용 Mock(Test Subclass) 컴포넌트
	// Protected 멤버들을 단위 테스트에서 접근할 수 있도록 using 선언으로 Public 개방합니다.
	class UHACF_CombatComponent_Accessor : public UHACF_CombatComponent
	{
	public:
	
		// Protected 메서드 개방
		using UHACF_CombatComponent::BeginPlay;
		using UHACF_CombatComponent::AbilityInputTagPressed;
		using UHACF_CombatComponent::AbilityInputTagReleased;
		using UHACF_CombatComponent::ProcessAbilityInput;
		
		// Protected 변수 개방
		using UHACF_CombatComponent::InputPressedSpecHandles;
		using UHACF_CombatComponent::InputHeldSpecHandles;
		using UHACF_CombatComponent::InputReleasedSpecHandles;
	};		
	
	TEST_CLASS(HACF_CombatInputQueueTest, "HyperActionCombat.Tests.CombatComponent.InputQueue")
	{
		FActorTestSpawner Spawner;
		APawn* DummyPawn;
		UHACF_CombatComponent_Accessor* CombatComponent;
		UAbilitySystemComponent* ASC;
		
		// 매 테스트 시작 전 실행되는 Setup 로직
		BEFORE_EACH()
		{
			// 1) 더미 폰 생성 (컴포넌트들을 부착할 기본 액터)
			DummyPawn = &Spawner.SpawnActor<APawn>();
			
			ASSERT_THAT(IsNotNull(DummyPawn));
			
			// 2) ASC 부착
			ASC = NewObject<UAbilitySystemComponent>(DummyPawn, TEXT("AbilitySystemComponent"));
			ASC->RegisterComponent();
			DummyPawn->AddInstanceComponent(ASC);
			
			// 3) 테스트용 CombatComponent 부착
			UHACF_CombatComponent* RawCombatComponent = NewObject<UHACF_CombatComponent>(DummyPawn, TEXT("CombatComponent"));
			RawCombatComponent->RegisterComponent();
			
			DummyPawn->AddInstanceComponent(RawCombatComponent);
		
			CombatComponent = static_cast<UHACF_CombatComponent_Accessor*>(RawCombatComponent);
			
			// BeginPlay 모사: ASC를 Owner로부터 정상적으로 캐싱하도록 함
			CombatComponent->InitializeWithASC(ASC, DummyPawn);
		}
				
		// 테스트 시나리오 1: Pressed 입력 처리
		// Tag 입력 시 Pressed 버퍼와 Held 버퍼에 Handle이 올바르게 큐잉되는가?
		TEST_METHOD(AbilityInputTagPressed_AddsHandlesToPressedAndHeldQueues)
		{
			// Arrange (준비)
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));
		
			// 테스트용 어빌리티 Spec 생성 (이 태그를 입력으로 받는 어빌리티 가정)
			FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
			Spec.GetDynamicSpecSourceTags().AddTag(TestTag);
			FGameplayAbilitySpecHandle GivenHandle = ASC->GiveAbility(Spec);
			
			// Act (실행)
			CombatComponent->AbilityInputTagPressed(TestTag);
			
			// Assert (검증)
			// Pressed 큐와 Held 큐에 어빌리티 핸들이 제대로 추가되었는지 확인합니다.
			ASSERT_THAT(IsTrue(CombatComponent->InputPressedSpecHandles.Contains(GivenHandle)));
			ASSERT_THAT(IsTrue(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));
		}
		
		// 테스트 시나리오 2: ProcessAbilityInput을 통한 어빌리티 활성화
		// 큐에 있는 어빌리티가 틱(Tick)을 거치며 활성화되고, Pressed 큐가 비워지는가?
		TEST_METHOD(ProcessAbilityInput_ActivatesAbility_AndClearsPressedQueue)
		{
			// Arrange (준비)
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));
			
			FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
			Spec.GetDynamicSpecSourceTags().AddTag(TestTag);
			FGameplayAbilitySpecHandle GivenHandle = ASC->GiveAbility(Spec);
			
			// 인위적으로 Pressed 상태 큐잉
			CombatComponent->AbilityInputTagPressed(TestTag);
			
			// Act (실행)
			CombatComponent->ProcessAbilityInput(0.1f, false);
			
			// Assert (검증)
			// 1) 어빌리티가 큐를 타고 성공적으로 활성화(Active) 되었는가?
			FGameplayAbilitySpec* FoundSpec = ASC->FindAbilitySpecFromHandle(GivenHandle);
			ASSERT_THAT(IsNotNull(FoundSpec));
			
			// (주의: 어빌리티의 CanActivate 요건이 다 갖춰졌다고 가정)
			ASSERT_THAT(IsTrue(FoundSpec->IsActive()));
			
			// 2) 처리가 끝난 후 Pressed 큐는 비워졌는가?
			ASSERT_THAT(IsTrue(CombatComponent->InputPressedSpecHandles.IsEmpty()));
		
			// 3) 키를 떼진 않았으므로 Held 큐에는 남아있어야 함
			ASSERT_THAT(IsTrue(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));
		}
		
		// 테스트 시나리오 3: Released 입력 처리 및 버퍼 이동
		// 키를 떼었을 때 Held 큐에서 제거되고, Released 큐에 삽입되는가?
		TEST_METHOD(AbilityInputTagReleased_MovesHandleToReleasedQueue)
		{
			// Arrange
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));
			
			FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
			Spec.GetDynamicSpecSourceTags().AddTag(TestTag);
			FGameplayAbilitySpecHandle GivenHandle = ASC->GiveAbility(Spec);
			
			// 눌렀다가 떼는 상황 모사
			CombatComponent->AbilityInputTagPressed(TestTag);
		
			// Act
			CombatComponent->AbilityInputTagReleased(TestTag);
			
			// Assert
			// Held 큐에서는 제거되었고, Released 큐에 무사히 안착했는지 검증
			ASSERT_THAT(IsFalse(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));
			ASSERT_THAT(IsTrue(CombatComponent->InputReleasedSpecHandles.Contains(GivenHandle)));
		}
		
		// 테스트 시나리오 4: ProcessAbilityInput이 Released 큐를 비우고 Input 해제 처리를 하는가?
		TEST_METHOD(ProcessAbilityInput_ReleasesAbility_AndClearsReleasedQueue)
		{
			// Arrange
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));
			
			FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
			Spec.GetDynamicSpecSourceTags().AddTag(TestTag);
			FGameplayAbilitySpecHandle GivenHandle = ASC->GiveAbility(Spec);
			
			CombatComponent->AbilityInputTagPressed(TestTag);
			CombatComponent->ProcessAbilityInput(0.1f, false); // 눌러서 일단 어빌리티 Active 상태로 만듦
		
			CombatComponent->AbilityInputTagReleased(TestTag); // 떼기
			
			// Act (실행: Released 큐를 소화함)
			CombatComponent->ProcessAbilityInput(0.1f, false);
			
			// Assert
			FGameplayAbilitySpec* FoundSpec = ASC->FindAbilitySpecFromHandle(GivenHandle);
			ASSERT_THAT(IsNotNull(FoundSpec));
		
			// ASC가 해당 어빌리티 스펙의 InputPressed 상태를 false로 잘 넘겼는지 확인
			ASSERT_THAT(IsFalse(FoundSpec->InputPressed));
		
			// 처리가 끝난 후 Released 큐는 비워졌는가?
			ASSERT_THAT(IsTrue(CombatComponent->InputReleasedSpecHandles.IsEmpty()));
		}

		TEST_METHOD(AbilityInputTagPressed_DoesNotQueueDuplicatePressesWhileHeld)
		{
			// Arrange
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));

			FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
			Spec.GetDynamicSpecSourceTags().AddTag(TestTag);
			FGameplayAbilitySpecHandle GivenHandle = ASC->GiveAbility(Spec);

			// Act: 같은 프레임에 중복 Press 입력
			CombatComponent->AbilityInputTagPressed(TestTag);
			CombatComponent->AbilityInputTagPressed(TestTag);

			// Assert
			ASSERT_THAT(AreEqual(1, CombatComponent->InputPressedSpecHandles.Num()));
			ASSERT_THAT(AreEqual(1, CombatComponent->InputHeldSpecHandles.Num()));
			ASSERT_THAT(IsTrue(CombatComponent->InputPressedSpecHandles.Contains(GivenHandle)));

			CombatComponent->ProcessAbilityInput(0.1f, false);
			CombatComponent->AbilityInputTagPressed(TestTag);

			ASSERT_THAT(IsTrue(CombatComponent->InputPressedSpecHandles.IsEmpty()));
			ASSERT_THAT(AreEqual(1, CombatComponent->InputHeldSpecHandles.Num()));
			ASSERT_THAT(IsTrue(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));
		}

		TEST_METHOD(AbilityInputTagReleased_DoesNotQueueDuplicateReleases)
		{
			// Arrange
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));

			FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
			Spec.GetDynamicSpecSourceTags().AddTag(TestTag);
			FGameplayAbilitySpecHandle GivenHandle = ASC->GiveAbility(Spec);
			
			CombatComponent->AbilityInputTagPressed(TestTag);

			// Act
			CombatComponent->AbilityInputTagReleased(TestTag);
			CombatComponent->AbilityInputTagReleased(TestTag);

			// Assert
			ASSERT_THAT(AreEqual(1, CombatComponent->InputReleasedSpecHandles.Num()));
			ASSERT_THAT(IsTrue(CombatComponent->InputReleasedSpecHandles.Contains(GivenHandle)));
			ASSERT_THAT(IsFalse(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));
		}

		TEST_METHOD(AbilityInputTagPressed_QueuesNewPressAfterRelease)
		{
			// Arrange
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));

			FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
			Spec.GetDynamicSpecSourceTags().AddTag(TestTag);
			FGameplayAbilitySpecHandle GivenHandle = ASC->GiveAbility(Spec);
			
			CombatComponent->AbilityInputTagPressed(TestTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);
			
			CombatComponent->AbilityInputTagReleased(TestTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);

			// Act
			CombatComponent->AbilityInputTagPressed(TestTag);

			// Assert
			ASSERT_THAT(AreEqual(1, CombatComponent->InputPressedSpecHandles.Num()));
			ASSERT_THAT(AreEqual(1, CombatComponent->InputHeldSpecHandles.Num()));
			ASSERT_THAT(IsTrue(CombatComponent->InputPressedSpecHandles.Contains(GivenHandle)));
			ASSERT_THAT(IsTrue(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));
		}

		TEST_METHOD(ProcessAbilityInput_DispatchesEachInputCallbackOnceToActiveAbility)
		{
			// Arrange
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));

			FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
			Spec.GetDynamicSpecSourceTags().AddTag(TestTag);
			FGameplayAbilitySpecHandle GivenHandle = ASC->GiveAbility(Spec);
			ASSERT_THAT(IsTrue(ASC->TryActivateAbility(GivenHandle)));

			FGameplayAbilitySpec* FoundSpec = ASC->FindAbilitySpecFromHandle(GivenHandle);
			ASSERT_THAT(IsNotNull(FoundSpec));
			
			UTestGameplayAbility* AbilityInstance = Cast<UTestGameplayAbility>(FoundSpec->GetPrimaryInstance());
			ASSERT_THAT(IsNotNull(AbilityInstance));
			
			const FPredictionKey PredictionKey = AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey();
			int32 ReplicatedInputPressedCount = 0;
			int32 ReplicatedInputReleasedCount = 0;
			const FDelegateHandle InputPressedDelegateHandle = ASC->AbilityReplicatedEventDelegate(
				EAbilityGenericReplicatedEvent::InputPressed, GivenHandle, PredictionKey).AddLambda(
				[&ReplicatedInputPressedCount]() { ++ReplicatedInputPressedCount; });
			const FDelegateHandle InputReleasedDelegateHandle = ASC->AbilityReplicatedEventDelegate(
				EAbilityGenericReplicatedEvent::InputReleased, GivenHandle, PredictionKey).AddLambda(
				[&ReplicatedInputReleasedCount]() { ++ReplicatedInputReleasedCount; });

			// Act: 중복 입력과 Held 상태의 다음 프레임 입력을 연속 처리
			CombatComponent->AbilityInputTagPressed(TestTag);
			CombatComponent->AbilityInputTagPressed(TestTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);
			
			CombatComponent->AbilityInputTagPressed(TestTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);
			
			CombatComponent->AbilityInputTagReleased(TestTag);
			CombatComponent->AbilityInputTagReleased(TestTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);
			
			CombatComponent->AbilityInputTagReleased(TestTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);

			// Assert
			ASSERT_THAT(AreEqual(1, AbilityInstance->GetInputPressedCount()));
			ASSERT_THAT(AreEqual(1, AbilityInstance->GetInputReleasedCount()));
			ASSERT_THAT(AreEqual(1, ReplicatedInputPressedCount));
			ASSERT_THAT(AreEqual(1, ReplicatedInputReleasedCount));

			ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed,
				GivenHandle, PredictionKey).Remove(InputPressedDelegateHandle);
			ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputReleased,
				GivenHandle, PredictionKey).Remove(InputReleasedDelegateHandle);
		}

		TEST_METHOD(ProcessAbilityInput_ReactivatesWhileInputActiveAbilityWhileHeld)
		{
			// Arrange
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));

			FGameplayAbilitySpec Spec(UTestWhileInputActiveGameplayAbility::StaticClass(), 1, -1, nullptr);
			Spec.GetDynamicSpecSourceTags().AddTag(TestTag);
			FGameplayAbilitySpecHandle GivenHandle = ASC->GiveAbility(Spec);

			CombatComponent->AbilityInputTagPressed(TestTag);
			CombatComponent->ProcessAbilityInput(0.1f, false);

			FGameplayAbilitySpec* FoundSpec = ASC->FindAbilitySpecFromHandle(GivenHandle);
			ASSERT_THAT(IsNotNull(FoundSpec));
			
			UTestGameplayAbility* AbilityInstance = Cast<UTestGameplayAbility>(FoundSpec->GetPrimaryInstance());
			ASSERT_THAT(IsNotNull(AbilityInstance));
			ASSERT_THAT(AreEqual(1, AbilityInstance->GetActivationCount()));
			
			AbilityInstance->EndTestAbility();

			// Act: Held 큐를 유지한 상태에서 다음 프레임 입력 처리
			CombatComponent->ProcessAbilityInput(0.1f, false);

			// Assert
			ASSERT_THAT(AreEqual(2, AbilityInstance->GetActivationCount()));
			ASSERT_THAT(IsTrue(CombatComponent->InputHeldSpecHandles.Contains(GivenHandle)));

		}
	};
}
