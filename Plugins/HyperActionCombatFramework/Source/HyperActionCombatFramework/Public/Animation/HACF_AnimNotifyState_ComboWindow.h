// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "HACF_AnimNotifyState_ComboWindow.generated.h"

class UHACF_CombatComponent;
/**
 * 
 */
UCLASS(meta = (DisplayName = "HACF Combo Window"))
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_AnimNotifyState_ComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual void BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	virtual void BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HACF|Combo")
	FGameplayTag WindowTag;
	
private:
	static UHACF_CombatComponent* FindCombatComponent(USkeletalMeshComponent* MeshComp);
};
