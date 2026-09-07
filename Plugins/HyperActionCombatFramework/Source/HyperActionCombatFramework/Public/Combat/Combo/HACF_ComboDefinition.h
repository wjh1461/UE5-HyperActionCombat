// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HACF_ComboTypes.h"
#include "Engine/DataAsset.h"
#include "HACF_ComboDefinition.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_ComboDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FName EntryNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (TitleProperty = "NodeId"))
	TArray<FHACF_ComboNode> Nodes;

	const FHACF_ComboNode* FindNode(FName NodeId) const;

	const FHACF_ComboNode* GetEntryNode() const;
};
