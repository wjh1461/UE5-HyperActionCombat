// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HACF_ActionWindowTypes.generated.h"

struct FHACF_BufferedInput;
struct FHACF_ActionWindowHandle;
struct FHACF_ActionWindowInputContext;

DECLARE_DELEGATE_RetVal_OneParam(bool, FHACF_ActionWindowInputHandler, const FHACF_ActionWindowInputContext&);

USTRUCT(BlueprintType)
struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_ActionWindowHandle
{
	GENERATED_BODY()
	
	FHACF_ActionWindowHandle() = default;

	bool IsValid() const
	{
		return Handle != INDEX_NONE;
	}

	void Invalidate()
	{
		Handle = INDEX_NONE;
	}

	bool operator==(const FHACF_ActionWindowHandle& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FHACF_ActionWindowHandle& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FHACF_ActionWindowHandle& Value)
	{
		return ::GetTypeHash(Value.Handle);
	}

	FString ToString() const
	{
		return IsValid() ? FString::FromInt(Handle) : TEXT("Invalid");
	}

private:
	explicit FHACF_ActionWindowHandle(int32 InHandle) : Handle(InHandle) { }

	static FHACF_ActionWindowHandle GenerateNewHandle();

	UPROPERTY()
	int32 Handle = INDEX_NONE;

	friend struct FHACF_ActionWindowContainer;
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FHACF_ActionWindow
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag WindowTag;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer AcceptedInputTags;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Priority = 0;
	
	UPROPERTY(BlueprintReadOnly)
	FHACF_ActionWindowHandle Handle;
};

/**
 * 외부 설정을 위한 구조체
 */
USTRUCT(BlueprintType)
struct FHACF_ActionWindowDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag WindowTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer AcceptedInputTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Priority = 0;
};

/**
 * 델리게이트 전용 Context 구조체
 */
USTRUCT(BlueprintType)
struct FHACF_ActionWindowInputContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FHACF_ActionWindowHandle WindowHandle;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag WindowTag;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly)
	double InputTime = 0.0;
};

/**
 * ActionWindow handler를 호출하기 전에 복사하는 dispatch 정보.
 * Handler가 실행 중 Window 컨테이너를 변경해도 이 값은 유효하다.
 */
struct FHACF_ActionWindowDispatchTarget
{
	FHACF_ActionWindowInputContext Context;
	FHACF_ActionWindowInputHandler Handler;
};

struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_ActionWindowContainer
{
	FHACF_ActionWindowHandle Open(const FHACF_ActionWindowDefinition& Definition);
	FHACF_ActionWindowHandle Open(const FHACF_ActionWindowDefinition& Definition, const UObject* Source, FHACF_ActionWindowInputHandler InputHandler);

	bool Close(FHACF_ActionWindowHandle Handle);
	int32 CloseAllFromSource(const UObject* Source);

	bool Contains(FHACF_ActionWindowHandle Handle) const;

	const FHACF_ActionWindow* FindHighestPriority() const;
	const FHACF_ActionWindow* FindHighestPriorityAccepting(const FGameplayTag& InputTag) const;
	bool FindDispatchTarget(const FGameplayTag& InputTag, double InputTime, FHACF_ActionWindowDispatchTarget& OutTarget) const;

	void Clear();

	bool IsEmpty() const
	{
		return Entries.IsEmpty();
	}

	int32 Num() const
	{
		return Entries.Num();
	}

private:
	struct FHACF_ActionWindowEntry
	{
		FHACF_ActionWindow Window;
		TOptional<FObjectKey> Source;
		FHACF_ActionWindowInputHandler Handler;
	};

	TArray<FHACF_ActionWindowEntry> Entries;
};
