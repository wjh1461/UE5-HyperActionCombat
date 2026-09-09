#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectKey.h"

struct FBranchingPointNotifyPayload;

// Queued IDs and Branching Point payloads identify different kinds of notify executions.
// NotifyState identity survives the engine copying an active FAnimNotifyEvent.
struct HYPERACTIONCOMBATFRAMEWORK_API FHACF_ComboWindowKey
{
	static FHACF_ComboWindowKey FromQueuedNotify(int32 NotifyInstanceId);
	static FHACF_ComboWindowKey FromBranchingPoint(const FBranchingPointNotifyPayload& Payload);
	
	bool IsValid() const;

	bool operator==(const FHACF_ComboWindowKey& Other) const
	{
		return NotifyId == Other.NotifyId && Mesh == Other.Mesh && NotifyState == Other.NotifyState;
	}

	friend uint32 GetTypeHash(const FHACF_ComboWindowKey& Key)
	{
		return HashCombine(HashCombine(GetTypeHash(Key.NotifyId), GetTypeHash(Key.Mesh)), GetTypeHash(Key.NotifyState));
	}

private:
	// Queued notify ID, or montage instance ID when Mesh/NotifyState are set.
	int32 NotifyId = INDEX_NONE;
	FObjectKey Mesh;
	FObjectKey NotifyState;
};
