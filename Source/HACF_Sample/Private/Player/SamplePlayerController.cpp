// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SamplePlayerController.h"
#include "Core/HACF_CombatComponent.h"
#include "GameFramework/Pawn.h"


void ASamplePlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	UHACF_CombatComponent* CombatComponent = GetPawn()->FindComponentByClass<UHACF_CombatComponent>();
	if (CombatComponent)
	{
		CombatComponent->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}
