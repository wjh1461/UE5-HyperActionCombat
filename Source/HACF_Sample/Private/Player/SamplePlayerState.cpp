// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SamplePlayerState.h"

#include "AbilitySystemComponent.h"

ASamplePlayerState::ASamplePlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
}
