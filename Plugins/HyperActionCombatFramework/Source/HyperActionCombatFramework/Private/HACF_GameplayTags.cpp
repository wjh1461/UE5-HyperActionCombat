#include "HACF_GameplayTags.h"

namespace HYPERACTIONCOMBATFRAMEWORK_API HACF_GameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "HACF.InitState.Spawned", "1. Actor와 Feature Component가 생성되고 등록됐다.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "HACF.InitState.DataAvailable", "2. 초기화에 필요한 참조와 복제 데이터가 준비됐다. 아직 실제 초기화 작업은 수행하지 않았다.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "HACF.InitState.DataInitialized", "3. ASC 연결, AbilitySet 부여, 로컬 입력 설정 등 실제 초기화 작업을 마쳤다.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "HACF.InitState.GameplayReady", "4. 일반 게임플레이에서 안전하게 사용할 수 있다.");
}
