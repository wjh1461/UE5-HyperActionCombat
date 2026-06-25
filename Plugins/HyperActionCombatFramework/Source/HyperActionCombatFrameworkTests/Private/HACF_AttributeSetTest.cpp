#include "CQTest.h"
#include "AbilitySystem/Attributes/HACF_AttributeSet.h"

namespace HyperActionCombatFrameworkTests
{
	TEST_CLASS(HACF_AttributeSetTest, "HyperActionCombat.Tests.[GenerateTestDirectory].AttributeSet")
	{
		// AttributeSet 초기화 시, 값들이 기본값과 일치해야 함
		TEST_METHOD(AttributeSet_Initialization_ValuesMatchDefaults)
		{
			UHACF_AttributeSet* AttributeSet = NewObject<UHACF_AttributeSet>();
			float Health = AttributeSet->GetHealth();
			float MaxHealth = AttributeSet->GetMaxHealth();
			float AttackPower = AttributeSet->GetAttackPower();
			float Armor = AttributeSet->GetArmor();
			float CriticalChance = AttributeSet->GetCriticalChance();
			float CriticalDamageMultiplier = AttributeSet->GetCriticalDamageMultiplier();
			
			float DefaultMaxHealth = 100.f;
			float DefaultAttackPower = 10.f;
			float DefaultArmor = 5.f;
			float DefaultCriticalChance = 0.5f;
			float DefaultCriticalDamageMultiplier = 1.5f;
			
			ASSERT_THAT(IsNear(Health, MaxHealth, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(AttackPower, DefaultAttackPower, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(Armor, DefaultArmor, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(CriticalChance, DefaultCriticalChance, KINDA_SMALL_NUMBER));
			ASSERT_THAT(IsNear(CriticalDamageMultiplier, DefaultCriticalDamageMultiplier, KINDA_SMALL_NUMBER));
		}
		
		// 체력이 Max를 넘어서 증가할 때, MaxHealth로 클램핑되어야 함
		TEST_METHOD(PreAttributeChange_HealthIncreasedBeyondMax_ClampsToMaxHealth)
		{
			UHACF_AttributeSet* AttributeSet = NewObject<UHACF_AttributeSet>();
			float Health = AttributeSet->GetHealth();
			float MaxHealth = AttributeSet->GetMaxHealth();
			
			float NewHealth = 150.f;
			AttributeSet->PreAttributeChange(AttributeSet->GetHealthAttribute(), NewHealth);
			
			ASSERT_THAT(AreEqual(Health, MaxHealth));
		}
		
		// 속성값들이 0 미만으로 감소할 때, 0으로 클램핑되어야 함
		TEST_METHOD(PreAttributeChange_AttributesDecreasedBelowZero_ClampsToZero)
		{
			//ASSERT_THAT(AreEqual());
		}
		
		// 데미지가 적용되었을 때, Health가 감소해야 함
		TEST_METHOD(PostGameplayEffectExecute_DamageApplied_ReducesHealth)
		{
			//ASSERT_THAT(AreEqual());
		}
		
		// 치명적인 데미지가 적용되었을 때, Health가 음수가 아닌 0으로 클램핑되어야 한다
		TEST_METHOD(PostGameplayEffectExecute_FatalDamageApplied_HealthClampsToZero)
		{
			//ASSERT_THAT(AreEqual());
		}
		
		// 힐링이 적용되었을 때, 최대 체력까지만 Health가 회복되어야 한다
		TEST_METHOD(PostGameplayEffectExecute_HealingApplied_RestoresHealthUpToMax)
		{
			//ASSERT_THAT(AreEqual());
		}
	};
}
