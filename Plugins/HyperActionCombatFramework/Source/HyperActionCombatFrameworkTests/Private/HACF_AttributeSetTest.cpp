#include "CQTest.h"
#include "AbilitySystem/Attributes/HACF_AttributeSet.h"
#include "Components/ActorTestSpawner.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(HACF_AttributeSetTest, "HyperActionCombat.Tests.AttributeSet")
	{
		FActorTestSpawner Spawner;
		
		// AttributeSet 초기화 시, 값들이 기본값과 일치해야 함
		TEST_METHOD(AttributeSet_Initialization_ValuesMatchDefaults)
		{
			float DefaultMaxHealth = 100.f;
			float DefaultAttackPower = 10.f;
			float DefaultArmor = 5.f;
			float DefaultCriticalChance = 0.5f;
			float DefaultCriticalDamageMultiplier = 1.5f;
			float DefaultPoise = 1.f;
			
			UHACF_AttributeSet* AttributeSet = NewObject<UHACF_AttributeSet>();
			AttributeSet->InitHealth(DefaultMaxHealth);
			AttributeSet->InitMaxHealth(DefaultMaxHealth);
			AttributeSet->InitAttackPower(DefaultAttackPower);
			AttributeSet->InitArmor(DefaultArmor);
			AttributeSet->InitCriticalChance(DefaultCriticalChance);
			AttributeSet->InitCriticalDamageMultiplier(DefaultCriticalDamageMultiplier);
			AttributeSet->InitPoise(DefaultPoise);
			
			ASSERT_THAT(IsNear(AttributeSet->GetHealth(), DefaultMaxHealth, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(AttributeSet->GetMaxHealth(), DefaultMaxHealth, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(AttributeSet->GetAttackPower(), DefaultAttackPower, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(AttributeSet->GetArmor(), DefaultArmor, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(AttributeSet->GetCriticalChance(), DefaultCriticalChance, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(AttributeSet->GetCriticalDamageMultiplier(), DefaultCriticalDamageMultiplier, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(AttributeSet->GetPoise(), DefaultPoise, KINDA_SMALL_NUMBER));
		}
		
		// 체력이 Max를 넘어서 증가할 때, MaxHealth로 클램핑되어야 함
		TEST_METHOD(PreAttributeChange_HealthIncreasedBeyondMax_ClampsToMaxHealth)
		{
			UHACF_AttributeSet* AttributeSet = NewObject<UHACF_AttributeSet>();
			AttributeSet->InitHealth(50.f);
			AttributeSet->InitMaxHealth(100.f);
			
			float NewHealth = 150.f;
			AttributeSet->PreAttributeChange(AttributeSet->GetHealthAttribute(), NewHealth);
			
			ASSERT_THAT(IsNear(NewHealth, AttributeSet->GetMaxHealth(), KINDA_SMALL_NUMBER));
		}
		
		// 속성값들이 0 미만으로 감소할 때, 0으로 클램핑되어야 함
		TEST_METHOD(PreAttributeChange_AttributesDecreasedBelowZero_ClampsToMinimum)
		{
			UHACF_AttributeSet* AttributeSet = NewObject<UHACF_AttributeSet>();
			AttributeSet->InitMaxHealth(100.f);
			
			// 체력은 0
			float NewHealth = -100.f;
			AttributeSet->PreAttributeChange(AttributeSet->GetHealthAttribute(), NewHealth);
			
			ASSERT_THAT(IsNear(NewHealth, 0.f, KINDA_SMALL_NUMBER));
			
			// 최대 체력은 1
			float NewMaxHealth = -100.f;
			AttributeSet->PreAttributeChange(AttributeSet->GetMaxHealthAttribute(), NewMaxHealth);
			
			ASSERT_THAT(IsNear(NewMaxHealth, 1.f, KINDA_SMALL_NUMBER));
			
			// 공격력은 0
			float NewAttackPower = -10.f;
			AttributeSet->PreAttributeChange(AttributeSet->GetAttackPowerAttribute(), NewAttackPower);
			
			ASSERT_THAT(IsNear(NewAttackPower, 0.f, KINDA_SMALL_NUMBER));
			
			// 방어력은 0
			float NewArmor = -10.f;
			AttributeSet->PreAttributeChange(AttributeSet->GetArmorAttribute(), NewArmor);
			
			ASSERT_THAT(IsNear(NewArmor, 0.f, KINDA_SMALL_NUMBER));
			
			// 크리티컬 확률은 0
			float NewCriticalChance = -10.f;
			AttributeSet->PreAttributeChange(AttributeSet->GetCriticalChanceAttribute(), NewCriticalChance);
			
			ASSERT_THAT(IsNear(NewCriticalChance, 0.f, KINDA_SMALL_NUMBER));
			
			// 크리티컬 데미지 배율은 1.f 이상
			float NewCriticalDamageMultiplier = -10.f;
			AttributeSet->PreAttributeChange(AttributeSet->GetCriticalDamageMultiplierAttribute(), NewCriticalDamageMultiplier);
			
			ASSERT_THAT(IsNear(NewCriticalDamageMultiplier, 1.f, KINDA_SMALL_NUMBER));
			
			// 강인도는 0
			float NewPoise = -10.f;
			AttributeSet->PreAttributeChange(AttributeSet->GetPoiseAttribute(), NewPoise);
			
			ASSERT_THAT(IsNear(NewPoise, 0.f, KINDA_SMALL_NUMBER));
		}
		
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

		// // 힐링이 적용되었을 때, 최대 체력까지만 Health가 회복되어야 한다
		// TEST_METHOD(PostGameplayEffectExecute_HealingApplied_RestoresHealthUpToMax)
		// {
		// 	//ASSERT_THAT(AreEqual());
		// }
	};
}
