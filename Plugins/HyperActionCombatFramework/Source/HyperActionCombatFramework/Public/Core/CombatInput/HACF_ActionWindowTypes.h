// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HACF_ActionWindowTypes.generated.h"

struct FHACF_BufferedInput;
struct FHACF_ActionWindowHandle;

DECLARE_DELEGATE_RetVal_ThreeParams(bool, FHACF_ActionWindowInputHandler, FHACF_ActionWindowHandle, const FHACF_ActionWindow&, const FHACF_BufferedInput&);

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
	int32 Priority;
	
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

struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_ActionWindowContainer
{
	FHACF_ActionWindowHandle Open(const FHACF_ActionWindowDefinition& Definition);

	bool Close(FHACF_ActionWindowHandle Handle);

	bool Contains(FHACF_ActionWindowHandle Handle) const;

	const FHACF_ActionWindow* FindHighestPriority() const;
	const FHACF_ActionWindow* FindHighestPriorityAccepting(const FGameplayTag& InputTag) const;

	void Clear();

	bool IsEmpty() const
	{
		return Windows.IsEmpty();
	}

	int32 Num() const
	{
		return Windows.Num();
	}

private:
	TArray<FHACF_ActionWindow> Windows;
};