// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HACF_GameplayAbility.h"
#include "Combat/Combo/HACF_ComboDefinition.h"
#include "Combat/Combo/HACF_ComboWindowKey.h"
#include "Core/CombatInput/HACF_ActionWindowTypes.h"
#include "HACF_CombatGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class HYPERACTIONCOMBATFRAMEWORK_API UHACF_CombatGameplayAbility : public UHACF_GameplayAbility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="HACF|Combat")
	void StartAttack(FName NodeId);
	
	void OpenComboWindow(int32 NotifyInstanceId, const FGameplayTag& WindowTag);
	void OpenComboWindow(const FHACF_ComboWindowKey& NotifyKey, const FGameplayTag& WindowTag);
	void CloseComboWindow(int32 NotifyInstanceId);
	void CloseComboWindow(const FHACF_ComboWindowKey& NotifyKey);
	// 현재 Ability가 생성하는 ActionWindow가 모두 콤보용이라는 전제
	void CloseAllComboWindows();
	
	// 예약된 전이를 실제로 적용
	void AdvanceCombo(const FHACF_ComboTransition* ComboTransition);
	
	void UnbindAttackMontageCallbacks();

protected:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
    void OnEndAbility(FName NotifyName) { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false); }
	
private:
	bool HandleComboInput(const FHACF_ActionWindowInputContext& Context);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HACF|ComboAttack", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UHACF_ComboDefinition> AttackComboDefinition;
	
	UPROPERTY(Transient)
	TObjectPtr<UPlayMoverMontageCallbackProxy> ActiveAttackMontageProxy;
	
	TMap<FHACF_ComboWindowKey, FHACF_ActionWindowHandle> ComboWindows;
	
	FName CurrentComboNodeId;
};
