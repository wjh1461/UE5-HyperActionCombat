#include "AbilitySystemComponent.h"
#include "CQTest.h"
#include "GameplayTagsManager.h"
#include "AbilitySystem/Abilities/HACF_GameplayAbilityBase.h"
#include "Character/HACF_HeroComponent.h"
#include "Components/ActorTestSpawner.h"
#include "Components/InputTestActions.h"
#include "Components/MapTestSpawner.h"
#include "NativeGameplayTags.h"
#include "Abilities/TestGameplayAbility.h"

namespace HyperActionCombatFrameworkTests
{
	// 1. 테스트 전용 Mock(Test Subclass) 컴포넌트
	// Protected 멤버들을 단위 테스트에서 접근할 수 있도록 using 선언으로 Public 개방합니다.
	class UHACF_HeroComponent_Accessor : public UHACF_HeroComponent
	{
	public:
	
		// Protected 메서드 개방
		using UHACF_HeroComponent::BeginPlay;
		using UHACF_HeroComponent::InitializePlayerInput;
		using UHACF_HeroComponent::AbilityInputTagPressed;
		using UHACF_HeroComponent::AbilityInputTagReleased;
		using UHACF_HeroComponent::ProcessAbilityInput;
		
		// Protected 변수 개방
		using UHACF_HeroComponent::InputPressedSpecHandles;
		using UHACF_HeroComponent::InputHeldSpecHandles;
		using UHACF_HeroComponent::InputReleasedSpecHandles;
	};		
	
	TEST_CLASS(HACF_EnhancedInputTest, "HyperActionCombat.Tests.EnhancedInput")
	{
		FActorTestSpawner Spawner;
		APawn* DummyPawn;
		UHACF_HeroComponent_Accessor* HeroComponent;
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
			
			// 3) 테스트용 HeroComponent 부착
			UHACF_HeroComponent* RawHeroComponent = NewObject<UHACF_HeroComponent>(DummyPawn, TEXT("HeroComponent"));
			RawHeroComponent->RegisterComponent();
			DummyPawn->AddInstanceComponent(RawHeroComponent);
		
			HeroComponent = static_cast<UHACF_HeroComponent_Accessor*>(RawHeroComponent);
			
			// BeginPlay 모사: ASC를 Owner로부터 정상적으로 캐싱하도록 함
			HeroComponent->BeginPlay();
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
			HeroComponent->AbilityInputTagPressed(TestTag);
			
			// Assert (검증)
			// Pressed 큐와 Held 큐에 어빌리티 핸들이 제대로 추가되었는지 확인합니다.
			ASSERT_THAT(IsTrue(HeroComponent->InputPressedSpecHandles.Contains(GivenHandle)));
			ASSERT_THAT(IsTrue(HeroComponent->InputHeldSpecHandles.Contains(GivenHandle)));
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
			HeroComponent->AbilityInputTagPressed(TestTag);
			
			// Act (실행)
			HeroComponent->ProcessAbilityInput(0.1f, false);
			
			// Assert (검증)
			// 1) 어빌리티가 큐를 타고 성공적으로 활성화(Active) 되었는가?
			FGameplayAbilitySpec* FoundSpec = ASC->FindAbilitySpecFromHandle(GivenHandle);
			ASSERT_THAT(IsNotNull(FoundSpec));
			
			// (주의: 어빌리티의 CanActivate 요건이 다 갖춰졌다고 가정)
			ASSERT_THAT(IsTrue(FoundSpec->IsActive()));
			
			// 2) 처리가 끝난 후 Pressed 큐는 비워졌는가?
			ASSERT_THAT(IsTrue(HeroComponent->InputPressedSpecHandles.IsEmpty()));
		
			// 3) 키를 떼진 않았으므로 Held 큐에는 남아있어야 함
			ASSERT_THAT(IsTrue(HeroComponent->InputHeldSpecHandles.Contains(GivenHandle)));
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
			HeroComponent->AbilityInputTagPressed(TestTag);
		
			// Act
			HeroComponent->AbilityInputTagReleased(TestTag);
			
			// Assert
			// Held 큐에서는 제거되었고, Released 큐에 무사히 안착했는지 검증
			ASSERT_THAT(IsFalse(HeroComponent->InputHeldSpecHandles.Contains(GivenHandle)));
			ASSERT_THAT(IsTrue(HeroComponent->InputReleasedSpecHandles.Contains(GivenHandle)));
		}
		
		// 테스트 시나리오 4: ProcessAbilityInput이 Released 큐를 비우고 Input 해제 처리를 하는가?
		TEST_METHOD(ProcessAbilityInput_ReleasesAbility_AndClearsReleasedQueue)
		{
			// Arrange
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("HACF.Input.TestTag"));
			
			FGameplayAbilitySpec Spec(UTestGameplayAbility::StaticClass(), 1, -1, nullptr);
			Spec.GetDynamicSpecSourceTags().AddTag(TestTag);
			FGameplayAbilitySpecHandle GivenHandle = ASC->GiveAbility(Spec);
			
			HeroComponent->AbilityInputTagPressed(TestTag);
			HeroComponent->ProcessAbilityInput(0.1f, false); // 눌러서 일단 어빌리티 Active 상태로 만듦
		
			HeroComponent->AbilityInputTagReleased(TestTag); // 떼기
			
			// Act (실행: Released 큐를 소화함)
			HeroComponent->ProcessAbilityInput(0.1f, false);
			
			// Assert
			FGameplayAbilitySpec* FoundSpec = ASC->FindAbilitySpecFromHandle(GivenHandle);
			ASSERT_THAT(IsNotNull(FoundSpec));
		
			// ASC가 해당 어빌리티 스펙의 InputPressed 상태를 false로 잘 넘겼는지 확인
			ASSERT_THAT(IsFalse(FoundSpec->InputPressed));
		
			// 처리가 끝난 후 Released 큐는 비워졌는가?
			ASSERT_THAT(IsTrue(HeroComponent->InputReleasedSpecHandles.IsEmpty()));
		}
	};
}
