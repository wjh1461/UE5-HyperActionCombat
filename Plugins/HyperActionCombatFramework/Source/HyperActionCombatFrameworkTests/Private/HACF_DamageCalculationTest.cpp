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
		
		UHACF_AttributeSet* SpawnTestAttributeSet()
		{
			AActor* DummyActor = &Spawner.SpawnActor<AActor>();
			UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(DummyActor);
			ASC->RegisterComponent();
			
			UHACF_AttributeSet* AttributeSet = NewObject<UHACF_AttributeSet>(DummyActor);
			ASC->AddAttributeSetSubobject(AttributeSet);
			ASC->InitAbilityActorInfo(DummyActor, DummyActor);
			
			return AttributeSet;
		}
		
		// 방어력이 0일 때 순수 데미지
		TEST_METHOD(DamageExecution_BaseAttack_AppliesRawDamage)
		{
			// 1. 테스트용 AttributeSet 생성
			UHACF_AttributeSet* AttributeSet = SpawnTestAttributeSet();
			
			// 2. 초기 스탯 셋업 (Health 100, AttackPower 30, Armor 0)
			AttributeSet->InitHealth(100.f);
			AttributeSet->InitMaxHealth(100.f);
			AttributeSet->InitAttackPower(30.f);
			AttributeSet->InitArmor(0.f);
			AttributeSet->InitDamage(0.f);
			
			// 3. 동적 GameplayEffect 생성 (30의 Damage Attribute 추가)
			UGameplayEffect* DamageEffect = NewObject<UGameplayEffect>(GetTransientPackage(), "TestDamage");
			DamageEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
			DamageEffect->Executions.Add(FGameplayEffectExecutionDefinition(UHACF_DamageExecution::StaticClass()));
			
			// 4. 자신에게 데미지 GE 적용
			UAbilitySystemComponent* ASC = AttributeSet->GetOwningAbilitySystemComponent();
			ASC->ApplyGameplayEffectToSelf(DamageEffect, 1.f, ASC->MakeEffectContext());
			
			// 5. 검증
			// 30(공격력) * 1(받는 피해 배율) = 30(최종 데미지), 100 - 30 = 70의 Health.
			ASSERT_THAT(IsNear(AttributeSet->GetHealth(), 70.f, KINDA_SMALL_NUMBER));
			// 계산이 끝난 후 최종 데미지 Attribute는 0으로 초기화 되어야 함.
			ASSERT_THAT(IsNear(AttributeSet->GetDamage(), 0.f, KINDA_SMALL_NUMBER));
		}
		
		// 방어력이 적용될 때 데미지
		TEST_METHOD(DamageExecution_WithArmor_CalculatesMitigatedDamage)
		{
			// 1. 테스트용 AttributeSet 생성
			UHACF_AttributeSet* AttributeSet = SpawnTestAttributeSet();
			
			// 2. 초기 스탯 셋업 (Health 100, AttackPower 30, Armor 20)
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
			UAbilitySystemComponent* ASC = AttributeSet->GetOwningAbilitySystemComponent();
			ASC->ApplyGameplayEffectToSelf(DamageEffect, 1.f, ASC->MakeEffectContext());
			
			// 5. 검증
			// 실제 적용되는 수식을 따라 Expected 값 도출
			constexpr float AttackerLevel = 1.f;
			constexpr float ArmorConstant = 800.f + (8.f * AttackerLevel);
    
			// 받는 피해 배율 = 808 / (20 + 808)
			constexpr float ExpectedMitigation = ArmorConstant / (20.f + ArmorConstant);
    
			// 최종 데미지 = 30 * 받는 피해 배율
			constexpr float ExpectedDamage = 30.f * ExpectedMitigation; 
    
			// 최종 기대 체력 = 100 - 최종 데미지
			constexpr float ExpectedHealth = 100.f - ExpectedDamage;
			ASSERT_THAT(IsNear(AttributeSet->GetHealth(), ExpectedHealth, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(AttributeSet->GetDamage(), 0.f, KINDA_SMALL_NUMBER));
		}
		
		// 크리티컬 데미지 배율 적용
		TEST_METHOD(DamageExecution_CriticalHit_CalculatesMultipliedDamage)
		{
			// 1. 테스트용 AttributeSet 생성
			UHACF_AttributeSet* AttributeSet = SpawnTestAttributeSet();
			
			// 2. 초기 스탯 셋업 (Health 100, AttackPower 30, 
			// CriticalChance = 1, CriticalDamageMultiplier = 1.5, Armor 0)
			AttributeSet->InitHealth(100.f);
			AttributeSet->InitMaxHealth(100.f);
			AttributeSet->InitAttackPower(30.f);
			AttributeSet->InitCriticalChance(1.f);
			AttributeSet->InitCriticalDamageMultiplier(1.5f);
			AttributeSet->InitArmor(0.f);
			AttributeSet->InitDamage(0.f);
			
			// 3. 동적 GameplayEffect 생성 (30의 Damage Attribute 추가)
			UGameplayEffect* DamageEffect = NewObject<UGameplayEffect>(GetTransientPackage(), "TestDamage");
			DamageEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
			DamageEffect->Executions.Add(FGameplayEffectExecutionDefinition(UHACF_DamageExecution::StaticClass()));
			
			// 4. 자신에게 데미지 GE 적용
			UAbilitySystemComponent* ASC = AttributeSet->GetOwningAbilitySystemComponent();
			ASC->ApplyGameplayEffectToSelf(DamageEffect, 1.f, ASC->MakeEffectContext());
			
			// 5. 검증
			// 30(공격력) * 1(받는 피해 배율) * 1.5(크리티컬 데미지 배율) = 45(최종 데미지)
			// 100 - 45 = 55의 Health.
			ASSERT_THAT(IsNear(AttributeSet->GetHealth(), 55.f, KINDA_SMALL_NUMBER));
			// 계산이 끝난 후 최종 데미지 Attribute는 0으로 초기화 되어야 함.
			ASSERT_THAT(IsNear(AttributeSet->GetDamage(), 0.f, KINDA_SMALL_NUMBER));
		}
	};
}
