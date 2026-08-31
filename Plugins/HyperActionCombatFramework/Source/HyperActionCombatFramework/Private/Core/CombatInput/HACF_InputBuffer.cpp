// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CombatInput/HACF_InputBuffer.h"

uint32 FHACF_InputBuffer::Push(const FHACF_BufferedInput& Input)
{
	FHACF_BufferedInput BufferedInput = Input;
	BufferedInput.Sequence = NextSequence++;

	// Sequence 0은 아직 버퍼에 입력되지 않은 입력값을 위해 예약되어 있음.
	if (NextSequence == 0)
	{
		NextSequence = 1;
	}

	Inputs.Add(BufferedInput);
	
	return BufferedInput.Sequence;
}

const FHACF_BufferedInput* FHACF_InputBuffer::Find() const
{
	return Inputs.IsEmpty() ? nullptr : &Inputs[0];
}

bool FHACF_InputBuffer::Consume(uint32 Sequence)
{
	const int32 InputIndex = Inputs.IndexOfByPredicate(
		[Sequence](const FHACF_BufferedInput& Input)
		{
			return Input.Sequence == Sequence;
		});

	if (InputIndex == INDEX_NONE)
	{
		return false;
	}

	Inputs.RemoveAt(InputIndex);
	
	return true;
}

void FHACF_InputBuffer::RemoveExpired(double CurrentTime)
{
	Inputs.RemoveAll(
		[CurrentTime](const FHACF_BufferedInput& Input)
		{
			return Input.IsExpired(CurrentTime);
		});
}

void FHACF_InputBuffer::Clear()
{
	Inputs.Reset();
}
