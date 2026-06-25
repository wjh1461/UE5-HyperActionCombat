// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HACF_AttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS_BASIC(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 기본 전투용 속성 집합
 * 체력, 최대 체력
 * 공격력, 방어력, 크리티컬 확률, 크리티컬 배율
 * 강인도
 */
UCLASS()
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UHACF_AttributeSet();
	
	ATTRIBUTE_ACCESSORS_BASIC(UHACF_AttributeSet, Health);
	ATTRIBUTE_ACCESSORS_BASIC(UHACF_AttributeSet, MaxHealth);
	
	ATTRIBUTE_ACCESSORS_BASIC(UHACF_AttributeSet, AttackPower);
	ATTRIBUTE_ACCESSORS_BASIC(UHACF_AttributeSet, Armor);
	
	ATTRIBUTE_ACCESSORS_BASIC(UHACF_AttributeSet, CriticalChance);
	ATTRIBUTE_ACCESSORS_BASIC(UHACF_AttributeSet, CriticalDamageMultiplier);
	
	ATTRIBUTE_ACCESSORS_BASIC(UHACF_AttributeSet, Poise);
	
	ATTRIBUTE_ACCESSORS_BASIC(UHACF_AttributeSet, Damage);
	ATTRIBUTE_ACCESSORS_BASIC(UHACF_AttributeSet, Healing);
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
private:
	UPROPERTY()
	FGameplayAttributeData Health;
	UPROPERTY()
	FGameplayAttributeData MaxHealth;
	
	UPROPERTY()
	FGameplayAttributeData AttackPower;
	UPROPERTY()
	FGameplayAttributeData Armor;
	
	UPROPERTY()
	FGameplayAttributeData CriticalChance;
	UPROPERTY()
	FGameplayAttributeData CriticalDamageMultiplier;
	
	UPROPERTY()
	FGameplayAttributeData Poise;
	
	// 메타 속성
	UPROPERTY()
	FGameplayAttributeData Damage;
	UPROPERTY()
	FGameplayAttributeData Healing;
	
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue);
};
