#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectKey.h"

struct FBranchingPointNotifyPayload;

// Queued ID와 Branching Point 페이로드는 서로 다른 종류의 노티파이 실행을 식별합니다.
// 엔진이 활성 FAnimNotifyEvent를 복사해도 NotifyState의 객체 식별 정보는 유지됩니다.
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
	// Queued 노티파이 ID이며, Mesh와 NotifyState가 설정된 경우에는 몽타주 인스턴스 ID입니다.
	int32 NotifyId = INDEX_NONE;
	FObjectKey Mesh;
	FObjectKey NotifyState;
};
