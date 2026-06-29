// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Executions/HACF_DamageExecution.h"
#include "AbilitySystem/Attributes/HACF_AttributeSet.h"

struct FHACFDamageStatics
{
	// Source
	FGameplayEffectAttributeCaptureDefinition AttackPowerDef;
	FGameplayEffectAttributeCaptureDefinition CriticalChanceDef;
	FGameplayEffectAttributeCaptureDefinition CriticalDamageMultiplierDef;
	
	// Target
	FGameplayEffectAttributeCaptureDefinition ArmorDef;
	
	FHACFDamageStatics()
	{
		AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
			UHACF_AttributeSet::GetAttackPowerAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			true);
		CriticalChanceDef = FGameplayEffectAttributeCaptureDefinition(
			UHACF_AttributeSet::GetCriticalChanceAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			true);
		CriticalDamageMultiplierDef = FGameplayEffectAttributeCaptureDefinition(
			UHACF_AttributeSet::GetCriticalDamageMultiplierAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			true);

		ArmorDef = FGameplayEffectAttributeCaptureDefinition(
			UHACF_AttributeSet::GetArmorAttribute(),
			EGameplayEffectAttributeCaptureSource::Target,
			true);
	}
};

static FHACFDamageStatics& DamageStatics()
{
	static FHACFDamageStatics HACFDamageStatics;
	return HACFDamageStatics;
}

UHACF_DamageExecution::UHACF_DamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalDamageMultiplierDef);
	
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
}

void UHACF_DamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                                   FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);
	
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	const FGameplayTagContainer* SourceTags= Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags= Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluateParameters, AttackPower);
	
	float CriticalChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalChanceDef, EvaluateParameters, CriticalChance);
	
	float CriticalDamageMultiplier = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalDamageMultiplierDef, EvaluateParameters, CriticalDamageMultiplier);
	
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParameters, TargetArmor);
	
	// 데미지 계산식
	// 공격력, 크리티컬 확률, 크리티컬 데미지 배율, 방어력
	// 스킬 계수
	// 최종 공격력 = (기본 공격력 + 무기 공격력) * (1 + 공격력 %)
	// 받는 피해 배율 = (800+8×캐릭 레벨) ÷ (적 방어력 + (800 + 8*캐릭 레벨))
	// 최종 데미지 = 최종 공격력 * 받는 피해 배율 * 크리티컬 데미지 배율
	
	const float AttackerLevel = 1.f;

	const float FinalAttackPower = AttackPower;
	const float FinalCriticalChance = CriticalChance;
	
	// 받는 피해 배율
	const float FinalTargetArmor = TargetArmor;
	const float ArmorConstant = 800.f + (8.f * AttackerLevel);
	const float ArmorMitigationMultiplier = ArmorConstant / (FMath::Max(FinalTargetArmor, 0.f) + ArmorConstant);

	float FinalCriticalDamageMultiplier = 1.f;
	if (FMath::FRand() <= FinalCriticalChance)
	{
		FinalCriticalDamageMultiplier = CriticalDamageMultiplier;
	}
	
	// 최종 데미지
	const float FinalDamage = FinalAttackPower * ArmorMitigationMultiplier * FinalCriticalDamageMultiplier;

	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UHACF_AttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, FinalDamage)
			);
	}
}
