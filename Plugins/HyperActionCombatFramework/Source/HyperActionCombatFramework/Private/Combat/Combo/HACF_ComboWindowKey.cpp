#include "Combat/Combo/HACF_ComboWindowKey.h"

#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Animation/AnimTypes.h"
#include "Components/SkeletalMeshComponent.h"

FHACF_ComboWindowKey FHACF_ComboWindowKey::FromQueuedNotify(int32 NotifyInstanceId)
{
	FHACF_ComboWindowKey Key;
	Key.NotifyId = NotifyInstanceId;
	return Key;
}

FHACF_ComboWindowKey FHACF_ComboWindowKey::FromBranchingPoint(const FBranchingPointNotifyPayload& Payload)
{
	if (!::IsValid(Payload.SkelMeshComponent) || !Payload.NotifyEvent
		|| !::IsValid(Payload.NotifyEvent->NotifyStateClass) || Payload.MontageInstanceID == INDEX_NONE)
	{
		return {};
	}

	FHACF_ComboWindowKey Key;
	Key.NotifyId = Payload.MontageInstanceID;
	Key.Mesh = FObjectKey(Payload.SkelMeshComponent);
	// FAnimNotifyEvent의 동등성 비교도 NotifyStateClass로 스테이트 노티파이를 식별합니다.
	Key.NotifyState = FObjectKey(Payload.NotifyEvent->NotifyStateClass);
	return Key;
}

bool FHACF_ComboWindowKey::IsValid() const
{
	return NotifyId != INDEX_NONE;
}
