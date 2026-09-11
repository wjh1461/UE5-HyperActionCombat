// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HACF_InputBufferTypes.h"

/**
 * 입력 버퍼 BufferedInput을 관리하기 위한 컨테이너.
 */
struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_InputBuffer
{
	uint32 Push(const FHACF_BufferedInput& Input);
	
	const FHACF_BufferedInput* Find() const;
	
	bool Consume(uint32 Sequence);
	
	void RemoveExpired(double CurrentTime);
	
	void Clear();

	bool IsEmpty() const { return Inputs.IsEmpty(); }
	
	int32 Num() const { return Inputs.Num(); }

private:
	TArray<FHACF_BufferedInput> Inputs;
	uint32 NextSequence = 1;
};
