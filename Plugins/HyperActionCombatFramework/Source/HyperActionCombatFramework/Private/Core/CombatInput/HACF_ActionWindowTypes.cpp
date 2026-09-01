// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CombatInput/HACF_ActionWindowTypes.h"

uint32 FHACF_ActionWindowContainer::Open(const FHACF_ActionWindowDefinition& Definition)
{
	FHACF_ActionWindow Window;
	Window.WindowTag = Definition.WindowTag;
	Window.AcceptedInputTags = Definition.AcceptedInputTags;
	Window.Priority = Definition.Priority;
	Window.WindowId = NextWindowId++;

	if (NextWindowId == 0)
	{
		NextWindowId = 1;
	}

	Windows.Add(Window);

	return Window.WindowId;
}

bool FHACF_ActionWindowContainer::Close(uint32 WindowId)
{
	if (WindowId == 0)
	{
		return false;
	}

	const int32 WindowIndex = Windows.IndexOfByPredicate(
		[WindowId](const FHACF_ActionWindow& Window)
		{
			return Window.WindowId == WindowId;
		});

	if (WindowIndex == INDEX_NONE)
	{
		return false;
	}

	// 배열 순서를 우선순위 동률 시 개방 순서로 사용할 수 있으므로
	// RemoveAtSwap 대신 RemoveAt을 사용한다.
	Windows.RemoveAt(WindowIndex);

	return true;
}

const FHACF_ActionWindow* FHACF_ActionWindowContainer::FindHighestPriority() const
{
	return nullptr;
}

void FHACF_ActionWindowContainer::Clear()
{
	Windows.Reset();
}
