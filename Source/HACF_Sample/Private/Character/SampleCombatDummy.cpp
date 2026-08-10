// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SampleCombatDummy.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/HACF_AttributeSet.h"


// Sets default values
ASampleCombatDummy::ASampleCombatDummy()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	CombatAttributeSet = CreateDefaultSubobject<UHACF_AttributeSet>(TEXT("CombatAttributeSet"));
}

// Called when the game starts or when spawned
void ASampleCombatDummy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASampleCombatDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASampleCombatDummy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

