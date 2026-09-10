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
	return Open(Definition, nullptr, FHACF_ActionWindowInputHandler());
}

FHACF_ActionWindowHandle FHACF_ActionWindowContainer::Open(const FHACF_ActionWindowDefinition& Definition, const UObject* Source, FHACF_ActionWindowInputHandler InputHandler)
{
	FHACF_ActionWindowEntry Entry;
	Entry.Window.WindowTag = Definition.WindowTag;
	Entry.Window.AcceptedInputTags = Definition.AcceptedInputTags;
	Entry.Window.Priority = Definition.Priority;
	Entry.Window.Handle = FHACF_ActionWindowHandle::GenerateNewHandle();
	Entry.Handler = MoveTemp(InputHandler);

	if (IsValid(Source))
	{
		Entry.Source.Emplace(Source);
	}

	const FHACF_ActionWindowHandle Handle = Entry.Window.Handle;
	Entries.Add(MoveTemp(Entry));

	return Handle;
}

bool FHACF_ActionWindowContainer::Close(FHACF_ActionWindowHandle Handle)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	const int32 EntryIndex = Entries.IndexOfByPredicate(
		[Handle](const FHACF_ActionWindowEntry& Entry)
		{
			return Entry.Window.Handle == Handle;
		});

	if (EntryIndex == INDEX_NONE)
	{
		return false;
	}

	// 동일 우선순위일 때 배열 순서가 개방 순서를 의미하므로
	// RemoveAtSwap 대신 RemoveAt을 사용한다.
	Entries.RemoveAt(EntryIndex);

	return true;
}

int32 FHACF_ActionWindowContainer::CloseAllFromSource(const UObject* Source)
{
	if (!Source)
	{
		return 0;
	}

	const FObjectKey SourceKey(Source);
	int32 ClosedCount = 0;

	for (int32 EntryIndex = Entries.Num() - 1; EntryIndex >= 0; --EntryIndex)
	{
		const FHACF_ActionWindowEntry& Entry = Entries[EntryIndex];
		if (!Entry.Source.IsSet() || Entry.Source.GetValue() != SourceKey)
		{
			continue;
		}

		Entries.RemoveAt(EntryIndex);
		++ClosedCount;
	}

	return ClosedCount;
}

bool FHACF_ActionWindowContainer::Contains(FHACF_ActionWindowHandle Handle) const
{
	if (!Handle.IsValid())
	{
		return false;
	}

	return Entries.ContainsByPredicate(
		[Handle](const FHACF_ActionWindowEntry& Entry)
		{
			return Entry.Window.Handle == Handle;
		});
}

const FHACF_ActionWindow* FHACF_ActionWindowContainer::FindHighestPriority() const
{
	const FHACF_ActionWindowEntry* Result = nullptr;

	for (const FHACF_ActionWindowEntry& Entry : Entries)
	{
		if (!Result || Entry.Window.Priority > Result->Window.Priority)
		{
			Result = &Entry;
		}
	}

	return Result ? &Result->Window : nullptr;
}

const FHACF_ActionWindow* FHACF_ActionWindowContainer::FindHighestPriorityAccepting(const FGameplayTag& InputTag) const
{
	if (!InputTag.IsValid())
	{
		return nullptr;
	}

	const FHACF_ActionWindowEntry* Result = nullptr;

	for (const FHACF_ActionWindowEntry& Entry : Entries)
	{
		if (!Entry.Window.AcceptedInputTags.HasTagExact(InputTag))
		{
			continue;
		}

		if (!Result || Entry.Window.Priority > Result->Window.Priority)
		{
			Result = &Entry;
		}
	}

	return Result ? &Result->Window : nullptr;
}

bool FHACF_ActionWindowContainer::FindDispatchTarget(const FGameplayTag& InputTag, double InputTime, FHACF_ActionWindowDispatchTarget& OutTarget) const
{
	if (!InputTag.IsValid())
	{
		return false;
	}

	const FHACF_ActionWindowEntry* Result = nullptr;

	for (const FHACF_ActionWindowEntry& Entry : Entries)
	{
		if (!Entry.Window.AcceptedInputTags.HasTagExact(InputTag))
		{
			continue;
		}

		// 같은 Priority라면 먼저 열린 Window를 유지한다.
		if (!Result || Entry.Window.Priority > Result->Window.Priority)
		{
			Result = &Entry;
		}
	}

	if (!Result)
	{
		return false;
	}

	OutTarget.Context.WindowHandle = Result->Window.Handle;
	OutTarget.Context.WindowTag = Result->Window.WindowTag;
	OutTarget.Context.InputTag = InputTag;
	OutTarget.Context.InputTime = InputTime;
	OutTarget.Handler = Result->Handler;
	
	return true;
}

void FHACF_ActionWindowContainer::Clear()
{
	Entries.Reset();
}
