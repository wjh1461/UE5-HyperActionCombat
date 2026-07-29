// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SamplePlayerController.h"
#include "Core/HACF_CombatComponent.h"
#include "GameFramework/Pawn.h"


void ASamplePlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (const APawn* Pawn = GetPawn())
	{
		if (UHACF_CombatComponent* CombatComponent = Pawn->FindComponentByClass<UHACF_CombatComponent>())
		{
			CombatComponent->ProcessAbilityInput(DeltaTime, bGamePaused);
		}
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}
