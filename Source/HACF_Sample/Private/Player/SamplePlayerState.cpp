// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SamplePlayerState.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/HACF_AttributeSet.h"

ASamplePlayerState::ASamplePlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	CombatAttributeSet = CreateDefaultSubobject<UHACF_AttributeSet>(TEXT("CombatAttributeSet"));
}
