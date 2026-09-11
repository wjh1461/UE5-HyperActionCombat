// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HACF_ComboTypes.generated.h"

struct FHACF_ActionWindowDefinition;

// 기본은 Immediate. AtSectionEnd, AtAdvanceNotify 사용은 추후 구현
UENUM(BlueprintType)
enum class EHACF_ComboTransitionMode : uint8
{
    // 즉시 전환
    Immediate UMETA(DisplayName = "Immediate"),
    
	// 같은 몽타주 안에서 현재 Section 끝에 연결.
	AtSectionEnd UMETA(DisplayName = "At Section End"),

	// ComboAdvance Notify 시점에 대상 재생 위치로 전환.
	AtAdvanceNotify UMETA(DisplayName = "At Advance Notify")
};

/** 
 * 현재 노드에서 다른 노드로 연결하는 규칙.
 */
USTRUCT(BlueprintType)
struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_ComboTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    FGameplayTag WindowTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    FGameplayTag InputTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    FName TargetNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    EHACF_ComboTransitionMode TransitionMode = EHACF_ComboTransitionMode::Immediate;

    // ASC가 모두 보유해야 하는 태그.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Conditions")
    FGameplayTagContainer RequiredTags;

    // ASC가 하나라도 보유하면 사용할 수 없음.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Conditions")
    FGameplayTagContainer BlockedTags;

    // 같은 윈도우·입력에 여러 전이가 해당할 때 사용합니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    int32 Priority = 0;

    bool Matches(const FGameplayTag& InWindowTag, const FGameplayTag& InInputTag, const FGameplayTagContainer& OwnedTags) const
    {
        return WindowTag == InWindowTag
            && InputTag == InInputTag
            && OwnedTags.HasAll(RequiredTags)
            && !OwnedTags.HasAny(BlockedTags);
    }
};

/** 
 * 노드에서 사용하는 입력 윈도우의 설정.
 */
USTRUCT(BlueprintType)
struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_ComboWindow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    FGameplayTag WindowTag;

    // CombatComponent의 다른 ActionWindow와 비교하는 우선순위.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    int32 Priority = 0;
};

/** 
 * 하나의 공격과 그 공격에서 가능한 연결을 정의.
 */
USTRUCT(BlueprintType)
struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_ComboNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    FName NodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Animation")
    TObjectPtr<UAnimMontage> Montage = nullptr;

    // None이면 몽타주 처음부터 재생.
    // AtSectionEnd 연결에 참여하는 노드는 명시적으로 지정.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Animation")
    FName StartSection = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Animation", meta = (ClampMin = "0.01"))
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    TArray<FHACF_ComboWindow> Windows;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    TArray<FHACF_ComboTransition> Transitions;

    const FHACF_ComboTransition* FindTransition(const FGameplayTag& WindowTag, const FGameplayTag& InputTag, const FGameplayTagContainer& OwnedTags) const;

    bool BuildActionWindowDefinition(const FGameplayTag& WindowTag, FHACF_ActionWindowDefinition& OutDefinition) const;
};
