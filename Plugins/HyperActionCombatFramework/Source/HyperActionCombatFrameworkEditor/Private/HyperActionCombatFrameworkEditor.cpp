// Copyright Epic Games, Inc. All Rights Reserved.

#include "HyperActionCombatFrameworkEditor.h"

#include "Combat/Combo/HACF_ComboTypes.h"
#include "Customizations/HACF_ComboNodeCustomization.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

DEFINE_LOG_CATEGORY_STATIC(LogHACFEditor, Log, All);

void FHyperActionCombatFrameworkEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	const FName ComboNodeTypeName = FHACF_ComboNode::StaticStruct()->GetFName();
	
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(ComboNodeTypeName,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FHACF_ComboNodeCustomization::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();
	
	UE_LOG(LogHACFEditor, Log, TEXT("Registered property customization for %s."), *ComboNodeTypeName.ToString());
}

void FHyperActionCombatFrameworkEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded(TEXT("PropertyEditor")))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout(FHACF_ComboNode::StaticStruct()->GetFName());
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}
}

IMPLEMENT_MODULE(FHyperActionCombatFrameworkEditorModule, HyperActionCombatFrameworkEditor)
