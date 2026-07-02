// Copyright Epic Games, Inc. All Rights Reserved.

#include "HyperActionCombatFramework.h"

#define LOCTEXT_NAMESPACE "FHyperActionCombatFrameworkModule"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"

void FHyperActionCombatFrameworkModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	// 플러그인 내부 Config/Tags 경로의 GameplayTags 등록
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("HyperActionCombatFramework"));
	if (Plugin.IsValid())
	{
		FString PluginBaseDir = Plugin->GetBaseDir();
		FString TagsDir = PluginBaseDir / TEXT("Config/Tags");
		
		UGameplayTagsManager::Get().AddTagIniSearchPath(TagsDir);
	}
}

void FHyperActionCombatFrameworkModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHyperActionCombatFrameworkModule, HyperActionCombatFramework)