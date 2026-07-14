// Fill out your copyright notice in the Description page of Project Settings.


#include "HACF_InitStateSubsystem.h"
#include "HACF_GameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemCollection.h"

void UHACF_InitStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	Collection.InitializeDependency<UGameFrameworkComponentManager>();
	
	UGameFrameworkComponentManager* Manager = GetGameInstance()->GetSubsystem<UGameFrameworkComponentManager>();
	
	check(Manager);
	
	Manager->RegisterInitState(HACF_GameplayTags::InitState_Spawned, false, FGameplayTag());
	Manager->RegisterInitState(HACF_GameplayTags::InitState_DataAvailable, false, HACF_GameplayTags::InitState_Spawned);
	Manager->RegisterInitState(HACF_GameplayTags::InitState_DataInitialized, false, HACF_GameplayTags::InitState_DataAvailable);
	Manager->RegisterInitState(HACF_GameplayTags::InitState_GameplayReady, false, HACF_GameplayTags::InitState_DataInitialized);
}
