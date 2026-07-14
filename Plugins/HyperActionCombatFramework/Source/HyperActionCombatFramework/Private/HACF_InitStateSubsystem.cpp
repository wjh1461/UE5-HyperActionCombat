// Fill out your copyright notice in the Description page of Project Settings.


#include "HACF_InitStateSubsystem.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemCollection.h"

void UHACF_InitStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	Collection.InitializeDependency<UGameFrameworkComponentManager>();
	
	UGameFrameworkComponentManager* Manager = GetGameInstance()->GetSubsystem<UGameFrameworkComponentManager>();
	
	check(Manager);
	
	const FGameplayTag Spawned = FGameplayTag::RequestGameplayTag(TEXT("HACF.InitState.Spawned"));
	const FGameplayTag DataAvailable = FGameplayTag::RequestGameplayTag(TEXT("HACF.InitState.DataAvailable"));
	const FGameplayTag DataInitialized = FGameplayTag::RequestGameplayTag(TEXT("HACF.InitState.DataInitialized"));
	const FGameplayTag GameplayReady = FGameplayTag::RequestGameplayTag(TEXT("HACF.InitState.GameplayReady"));
	
	Manager->RegisterInitState(Spawned, false, FGameplayTag());
	Manager->RegisterInitState(DataAvailable, false, Spawned);
	Manager->RegisterInitState(DataInitialized, false, DataAvailable);
	Manager->RegisterInitState(GameplayReady, false, DataInitialized);
}
