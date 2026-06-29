#include "CQTest.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AbilitySystem/Attributes/HACF_AttributeSet.h"
#include "AbilitySystem/Executions/HACF_DamageExecution.h"
#include "Components/ActorTestSpawner.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(HACF_DamageCalculationTest, "HyperActionCombat.Tests.DamageCalculation")
	{
		FActorTestSpawner Spawner;
		
		// 데미지가 적용되었을 때, 체력이 감소해야 함.
		TEST_METHOD(PostGameplayEffectExecute_DamageApplied_ReducesHealthAndResetsDamage)
		{
			// 1. 임시 액터 및 ASC 세팅
			AActor* DummyActor = &Spawner.SpawnActor<AActor>();
			UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(DummyActor);
			ASC->RegisterComponent();
			
			UHACF_AttributeSet* AttributeSet = NewObject<UHACF_AttributeSet>(DummyActor);
			ASC->AddAttributeSetSubobject(AttributeSet);
			ASC->InitAbilityActorInfo(DummyActor, DummyActor);
			
			// 2. 초기 스탯 셋업 (Health 100, Armor 0)
			AttributeSet->InitHealth(100.f);
			AttributeSet->InitMaxHealth(100.f);
			AttributeSet->InitArmor(0.f);
			AttributeSet->InitDamage(0.f);
			
			// 3. 동적 GameplayEffect 생성 (30의 Damage Attribute 추가)
			UGameplayEffect* DamageEffect = NewObject<UGameplayEffect>(GetTransientPackage(), "TestDamage");
			DamageEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
			
			FGameplayModifierInfo ModifierInfo;
			ModifierInfo.Attribute = UHACF_AttributeSet::GetDamageAttribute();
			ModifierInfo.ModifierOp = EGameplayModOp::Additive;
			ModifierInfo.ModifierMagnitude = FScalableFloat(30.f);
			DamageEffect->Modifiers.Add(ModifierInfo);
			
			// 4. 자신에게 데미지 GE 적용
			ASC->ApplyGameplayEffectToSelf(DamageEffect, 1.f, ASC->MakeEffectContext());
			
			// 5. 검증
			// 30 최종 데미지, 100 - 30 = 70의 Health.
			ASSERT_THAT(IsNear(AttributeSet->GetHealth(), 70.f, KINDA_SMALL_NUMBER));
			// 계산이 끝난 후 최종 데미지 Attribute는 0으로 초기화 되어야 함.
			ASSERT_THAT(IsNear(AttributeSet->GetDamage(), 0.f, KINDA_SMALL_NUMBER));
		}

		// 데미지가 적용되었을 때, 방어력을 반영하여 체력이 감소해야 함.
		TEST_METHOD(PostGameplayEffectExecute_DamageAppliedWithArmor_MitigatesDamageAndReducesHealth)
		{
			// 1. 임시 액터 및 ASC 세팅
			AActor* DummyActor = &Spawner.SpawnActor<AActor>();
			UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(DummyActor);
			ASC->RegisterComponent();
			
			UHACF_AttributeSet* AttributeSet = NewObject<UHACF_AttributeSet>(DummyActor);
			ASC->AddAttributeSetSubobject(AttributeSet);
			ASC->InitAbilityActorInfo(DummyActor, DummyActor);
			
			// 2. 초기 스탯 셋업 (Health 100, Armor 20)
			AttributeSet->InitHealth(100.f);
			AttributeSet->InitMaxHealth(100.f);
			AttributeSet->InitAttackPower(30.f);
			AttributeSet->InitArmor(20.f);
			AttributeSet->InitDamage(0.f);
			
			// 3. 동적 GameplayEffect 생성 (30의 Damage Attribute 추가)
			UGameplayEffect* DamageEffect = NewObject<UGameplayEffect>(GetTransientPackage(), "TestDamage");
			DamageEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
			DamageEffect->Executions.Add(FGameplayEffectExecutionDefinition(UHACF_DamageExecution::StaticClass()));
			
			// 4. 자신에게 데미지 GE 적용
			ASC->ApplyGameplayEffectToSelf(DamageEffect, 1.f, ASC->MakeEffectContext());
			
			// 5. 검증
			// 30(공격력) - 20(방어력) = 10(최종 데미지), 100 - 10 = 90의 Health.
			ASSERT_THAT(IsNear(AttributeSet->GetHealth(), 90.f, KINDA_SMALL_NUMBER));
			// 계산이 끝난 후 최종 데미지 Attribute는 0으로 초기화 되어야 함.
			ASSERT_THAT(IsNear(AttributeSet->GetDamage(), 0.f, KINDA_SMALL_NUMBER));
		}
		
		// 치명적인 데미지가 적용되었을 때, Health가 음수가 아닌 0으로 클램핑되어야 한다
		TEST_METHOD(PostGameplayEffectExecute_FatalDamageApplied_HealthClampsToZero)
		{
			AActor* DummyActor = &Spawner.SpawnActor<AActor>();
			UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(DummyActor);
			ASC->RegisterComponent();
			
			UHACF_AttributeSet* AttributeSet = NewObject<UHACF_AttributeSet>(DummyActor);
			ASC->AddAttributeSetSubobject(AttributeSet);
			ASC->InitAbilityActorInfo(DummyActor, DummyActor);
			
			AttributeSet->InitHealth(10.f); // 체력을 10으로 아주 낮게 설정
			AttributeSet->InitMaxHealth(100.f);
			AttributeSet->InitDamage(0.f);
			
			UGameplayEffect* DamageEffect = NewObject<UGameplayEffect>(GetTransientPackage(), "FatalDamage");
			DamageEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
			
			FGameplayModifierInfo ModifierInfo;
			ModifierInfo.Attribute = UHACF_AttributeSet::GetDamageAttribute();
			ModifierInfo.ModifierOp = EGameplayModOp::Additive;
			ModifierInfo.ModifierMagnitude = FScalableFloat(200.f); // 200의 치명타
			DamageEffect->Modifiers.Add(ModifierInfo);
			
			ASC->ApplyGameplayEffectToSelf(DamageEffect, 1.f, ASC->MakeEffectContext());
			
			// 10 - 200 = -190 이지만, 0으로 클램핑 되어야 함
			ASSERT_THAT(IsNear(AttributeSet->GetHealth(), 0.f, KINDA_SMALL_NUMBER));
			// 계산이 끝난 후 최종 데미지 Attribute는 0으로 초기화 되어야 함.
			ASSERT_THAT(IsNear(AttributeSet->GetDamage(), 0.f, KINDA_SMALL_NUMBER));
		}
	};
}
