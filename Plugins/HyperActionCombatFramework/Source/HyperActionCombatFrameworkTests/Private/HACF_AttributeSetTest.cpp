#include "CQTest.h"
#include "AbilitySystem/Attributes/HACF_AttributeSet.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(HACF_AttributeSetTest, "HyperActionCombat.Tests.AttributeSet")
	{
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
		
		// // 데미지가 적용되었을 때, Health가 감소해야 함
		// TEST_METHOD(PostGameplayEffectExecute_DamageApplied_ReducesHealth)
		// {
		// 	//ASSERT_THAT(AreEqual());
		// }
		//
		// // 치명적인 데미지가 적용되었을 때, Health가 음수가 아닌 0으로 클램핑되어야 한다
		// TEST_METHOD(PostGameplayEffectExecute_FatalDamageApplied_HealthClampsToZero)
		// {
		// 	//ASSERT_THAT(AreEqual());
		// }
		//
		// // 힐링이 적용되었을 때, 최대 체력까지만 Health가 회복되어야 한다
		// TEST_METHOD(PostGameplayEffectExecute_HealingApplied_RestoresHealthUpToMax)
		// {
		// 	//ASSERT_THAT(AreEqual());
		// }
	};
}
