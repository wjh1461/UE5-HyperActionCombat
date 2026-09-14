// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HACF_TargetingTypes.generated.h"

class UHACF_TargetingConfig;

// 호출자가 제공하는 위치와 선택적 방향
USTRUCT(BlueprintType)
struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_TargetingProvidedLocation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	bool bHasDirection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (EditCondition = "bHasDirection"))
	FVector Direction = FVector::ForwardVector;
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FHACF_TargetingRequest
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	TObjectPtr<UHACF_TargetingConfig> Config;
	
	// 대상 검색의 출발 Actor
	UPROPERTY()
	TWeakObjectPtr<AActor> SourceActor;
	
	// 요청 수명과 정리를 담당하는 논리적 소유자. 일반 검색에서는 생략 가능, 락온 획득에서는 필수
	UPROPERTY()
	TWeakObjectPtr<UObject> RequestOwner;
	
	// 월드 좌표(cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    FVector Origin = FVector::ZeroVector;

    // 방향이 필요한 Task에서 검증 후 정규화
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    FVector AimDirection = FVector::ForwardVector;

    // 점수 계산에 사용할 우선 대상
    UPROPERTY()
    TWeakObjectPtr<AActor> PreferredTarget;

    // ProvidedActors Selection의 입력
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ProvidedActors;

    // 모든 Selection에 공통으로 적용할 제외 목록
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ExcludedActors;
	
	// SelectProvidedLocation의 입력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Location")
	bool bHasProvidedLocation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Location", meta = (EditCondition = "bHasProvidedLocation"))
	FHACF_TargetingProvidedLocation ProvidedLocation;

    // 이번 요청의 의미·실행 조건을 전달하는 태그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    FGameplayTagContainer ContextTags;

    // 미지정이면 Config.DefaultRange 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Overrides")
    bool bOverrideRange = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Overrides", meta = (EditCondition = "bOverrideRange", ClampMin = "0.001"))
    float RangeOverride = 500.0f;

    // 미지정이면 Config.MaxTargets 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Overrides")
    bool bOverrideMaxTargets = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Overrides", meta = (EditCondition = "bOverrideMaxTargets", ClampMin = "1"))
    int32 MaxTargetsOverride = 1;

    // Source ASC 등이 SourceActor 외부에 있을 때 사용하는 선택적 전투 데이터 제공 Actor
    UPROPERTY()
    TWeakObjectPtr<AActor> CombatDataProviderActor;
};

/**
 * 
 */
struct FHACF_TargetingCandidate
{
	
};

/**
 * 
 */
struct FHACF_TargetingExecutionContext
{
	
};

/**
 * 
 */
struct FHACF_TargetingResult
{
	
};

UENUM(BlueprintType)
enum class EHACF_TargetCategory : uint8
{
	Actor UMETA(DisplayName = "Actor"),
	Location UMETA(DisplayName = "Location"),
};

UENUM(BlueprintType)
enum class EHACF_TargetSortMode : uint8
{
	ScoreDescending UMETA(DisplayName = "Score Descending"),
	ScoreAscending UMETA(DisplayName = "Score Ascending"),
};

UENUM(BlueprintType)
enum class EHACF_TargetingResultStatus : uint8
{
	Success,            // 최종 유효 후보가 하나 이상
	NoCandidates,       // 정상 실행했지만 유효 후보 없음
	InvalidRequest,     // Config·입력·설정 검증 실패
	ServiceUnavailable, // 서비스가 연결되지 않았거나 유효하지 않음
	NotReady,           // 서비스는 있으나 실행 준비가 되지 않음
	Failed,             // Task 실행 또는 계산 실패
	Cancelled           // 후속 취소 API에서 사용
};

// 개별 Task의 실행 결과
UENUM(BlueprintType)
enum class EHACF_TargetingTaskStatus : uint8
{
	Success, // 해당 단계 작업 완료
	Skipped, // 선택적 실행 조건이 불일치하여 건너뜀
	Failed   // 전체 요청을 실패로 종료
};

// Task의 파이프라인 단계
UENUM(BlueprintType)
enum class EHACF_TargetingTaskStage : uint8
{
	Selection,
	Filter,
	Score
};