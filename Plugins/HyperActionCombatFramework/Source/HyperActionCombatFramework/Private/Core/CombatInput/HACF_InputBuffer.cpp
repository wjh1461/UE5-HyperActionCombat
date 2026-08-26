// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CombatInput/HACF_InputBuffer.h"

void FHACF_InputBuffer::Push(const FHACF_BufferedInput& Input)
{
}

const FHACF_BufferedInput* FHACF_InputBuffer::Find() const
{
	return nullptr;
}

bool FHACF_InputBuffer::Consume(uint32 Sequence)
{
	return false;
}

void FHACF_InputBuffer::RemoveExpired(double CurrentTime)
{
}

void FHACF_InputBuffer::Clear()
{
}
