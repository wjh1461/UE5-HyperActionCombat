# HyperActionCombatFramework

Unreal Engine 5.8용 액션 전투 프레임워크 플러그인입니다. Gameplay Ability System, Enhanced Input, Mover를 기반으로 전투 입력 버퍼, 콤보, 타게팅 및 데미지 계산을 제공합니다. 현재 베타 단계입니다.

| 모듈 | 유형 | 역할 |
| --- | --- | --- |
| `HyperActionCombatFramework` | Runtime | 전투 프레임워크 공개 API 및 구현 |
| `HyperActionCombatFrameworkEditor` | Editor | 콤보 노드 Details 커스터마이징 |
| `HyperActionCombatFrameworkTests` | UncookedOnly | CQTest 기반 자동화 테스트 |

프로젝트 요구 사항, 샘플 실행 방법, 핵심 클래스와 테스트 명령은 [프로젝트 README](../../README.md)를 참고하세요.

캐릭터에 적용하는 방법은 [캐릭터 연결 안내](../../README.md#캐릭터에-연결하기)에 정리되어 있습니다. 타게팅은 `IHACF_TargetingService` 인터페이스를 통해 요청하며, 기본 구현인 `UHACF_TargetingComponent`가 `UHACF_TargetingExecutor`로 파이프라인을 실행합니다.

일부 자동화 테스트는 샘플 프로젝트의 `/Game` 맵과 데이터 에셋을 로드합니다. 플러그인만 다른 프로젝트로 옮기는 경우 [테스트 의존성과 검증 범위](../../README.md#테스트-의존성과-검증-범위)를 확인하세요.
