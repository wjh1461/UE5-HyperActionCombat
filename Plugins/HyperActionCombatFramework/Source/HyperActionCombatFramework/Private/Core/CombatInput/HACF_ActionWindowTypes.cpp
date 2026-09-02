// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CombatInput/HACF_ActionWindowTypes.h"

namespace
{
	uint32 GNextActionWindowHandle = 0;
}

FHACF_ActionWindowHandle FHACF_ActionWindowHandle::GenerateNewHandle()
{
	// Window 생성은 게임 스레드에서만 수행한다.
	check(IsInGameThread());

	++GNextActionWindowHandle;

	// 0은 invalid 값이므로 overflow 시 건너뛴다.
	if (GNextActionWindowHandle == 0)
	{
		++GNextActionWindowHandle;
	}

	return FHACF_ActionWindowHandle(GNextActionWindowHandle);
}

FHACF_ActionWindowHandle FHACF_ActionWindowContainer::Open(const FHACF_ActionWindowDefinition& Definition)
{
	FHACF_ActionWindow Window;
	Window.WindowTag = Definition.WindowTag;
	Window.AcceptedInputTags = Definition.AcceptedInputTags;
	Window.Priority = Definition.Priority;
	Window.Handle = FHACF_ActionWindowHandle::GenerateNewHandle();

	Windows.Add(Window);

	return Window.Handle;
}

bool FHACF_ActionWindowContainer::Close(FHACF_ActionWindowHandle Handle)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	const int32 WindowIndex = Windows.IndexOfByPredicate(
		[Handle](const FHACF_ActionWindow& Window)
		{
			return Window.Handle == Handle;
		});

	if (WindowIndex == INDEX_NONE)
	{
		return false;
	}

	// 동일 우선순위일 때 배열 순서가 개방 순서를 의미하므로
	// RemoveAtSwap 대신 RemoveAt을 사용한다.
	Windows.RemoveAt(WindowIndex);

	return true;
}

bool FHACF_ActionWindowContainer::Contains(FHACF_ActionWindowHandle Handle) const
{
	if (!Handle.IsValid())
	{
		return false;
	}

	return Windows.ContainsByPredicate(
		[Handle](const FHACF_ActionWindow& Window)
		{
			return Window.Handle == Handle;
		});
}

const FHACF_ActionWindow* FHACF_ActionWindowContainer::FindHighestPriority() const
{
	const FHACF_ActionWindow* Result = nullptr;

	for (const FHACF_ActionWindow& Window : Windows)
	{
		// 같은 Priority라면 먼저 열린 Window를 유지한다.
		if (!Result || Window.Priority > Result->Priority)
		{
			Result = &Window;
		}
	}

	return Result;
}

const FHACF_ActionWindow* FHACF_ActionWindowContainer::FindHighestPriorityAccepting(const FGameplayTag& InputTag) const
{
	if (!InputTag.IsValid())
	{
		return nullptr;
	}

	const FHACF_ActionWindow* Result = nullptr;

	for (const FHACF_ActionWindow& Window : Windows)
	{
		if (!Window.AcceptedInputTags.HasTagExact(InputTag))
		{
			continue;
		}

		if (!Result || Window.Priority > Result->Priority)
		{
			Result = &Window;
		}
	}

	return Result;
}

void FHACF_ActionWindowContainer::Clear()
{
	Windows.Reset();
}
