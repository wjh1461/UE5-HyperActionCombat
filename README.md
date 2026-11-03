# HyperActionCombat

Unreal Engine 5.8 기반의 C++ 액션 전투 샘플 프로젝트입니다. **HyperActionCombatFramework (HACF)** 플러그인에서 전투 기능을 제공하고, `HACF_Sample` 모듈과 Blueprint 에셋에서 이를 연결해 사용합니다.

Gameplay Ability System(GAS), Enhanced Input, Mover를 기반으로 입력 처리, 어빌리티 활성화, 콤보, 타게팅, 데미지 계산을 구성합니다. 플러그인은 현재 베타 단계입니다.

## 주요 기능

- **데이터 기반 캐릭터 구성**: `UHACF_PawnData`에서 Pawn 클래스, AbilitySet, InputConfig, 기본 Input Mapping Context를 지정합니다.
- **초기화 상태 관리**: ModularGameplay의 초기화 상태를 이용해 HeroComponent와 CombatComponent를 준비합니다.
- **태그 기반 입력 처리**: Enhanced Input 액션을 Gameplay Tag와 연결하고, 전투 입력 큐와 입력 버퍼로 어빌리티 입력을 처리합니다.
- **콤보 및 입력 윈도우**: ComboDefinition의 노드와 전이 조건을 사용하고, ComboWindow AnimNotifyState로 콤보 입력 구간을 제어합니다. 전투 어빌리티는 Mover 몽타주 재생과 연결됩니다.
- **타게팅 파이프라인**: Selector → Filter → Scorer 순서로 후보를 수집, 필터링, 평가합니다. 구체 오버랩, 거리 및 전방 부채꼴 필터, 조준 방향과 거리 기반 평가를 제공합니다.
- **전투 속성과 데미지**: 체력, 최대 체력, 공격력, 방어력, 치명타 확률·배율, 강인도 및 Damage/Healing 메타 속성을 제공합니다. DamageExecution에서 방어력 감소와 치명타를 반영합니다.
- **자동화 테스트**: CQTest와 CQTestEnhancedInput으로 입력, 초기화, 콤보, 타게팅, 속성 및 데미지 계산을 검증합니다.

## 요구 사항

- Unreal Engine **5.8** (`HACF_Sample.uproject`의 EngineAssociation 기준)
- Windows C++ 개발 환경과 해당 엔진 버전에 맞는 Visual Studio 및 Windows SDK
- Git 및 **Git LFS**: `.uasset`, `.umap` 등의 바이너리 콘텐츠는 LFS로 관리합니다.
- 프로젝트와 플러그인에 선언된 의존 플러그인: `GameplayAbilities`, `EnhancedInput`, `ModularGameplay`, `Mover`, `CQTest`, `CQTestEnhancedInput`

사용하는 엔진 설치에서 위 플러그인을 사용할 수 있어야 합니다. 누락된 플러그인이 있다면 프로젝트를 열거나 빌드하기 전에 설치 구성을 확인하세요.

## 시작하기

1. 저장소를 받은 뒤 저장소 루트에서 LFS 콘텐츠를 내려받습니다.

   ```powershell
   git lfs install
   git lfs pull
   ```

2. `HACF_Sample.uproject`를 UE 5.8에 연결하고 프로젝트 파일을 생성합니다.
3. IDE에서 **Development Editor / Win64** 구성으로 `HACF_SampleEditor` 타깃을 빌드합니다.
4. 프로젝트를 에디터로 열고 기본 맵 `/Game/Maps/L_CombatTest`에서 Play In Editor를 실행합니다.

샘플에는 `BP_Hero`, `BP_CombatDummy`, 일반 공격·강공격·회피 어빌리티, 콤보 및 타게팅 설정 에셋이 포함되어 있습니다. 실제 키 바인딩은 `/Game/Data/Input/Mappings/IMC_Combat`, 입력 태그 연결은 `/Game/Data/Input/InputData_CombatHero`에서 확인할 수 있습니다.

### 명령줄 빌드

다음은 Windows PowerShell 예시입니다. 엔진과 프로젝트 경로는 로컬 환경에 맞게 변경하세요.

```powershell
$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
$ProjectPath = Join-Path (Get-Location).Path 'HACF_Sample.uproject'

& "$EngineRoot\Engine\Build\BatchFiles\Build.bat" HACF_SampleEditor Win64 Development "-Project=$ProjectPath" -WaitMutex
```

## 프로젝트 구조

```text
Plugins/HyperActionCombatFramework/
├─ Source/HyperActionCombatFramework/        # 런타임 전투 프레임워크
│  ├─ Public/                               # 공개 API
│  └─ Private/                              # 구현
├─ Source/HyperActionCombatFrameworkEditor/  # 콤보 노드 Details 커스터마이징
├─ Source/HyperActionCombatFrameworkTests/   # CQTest 자동화 테스트
└─ Config/Tags/HACF_Tags.ini                 # 플러그인 Gameplay Tag 정의
Source/HACF_Sample/                         # 샘플 Pawn, PlayerState, Controller, Dummy
Content/
├─ Combat/                                 # 전투 어빌리티, 효과, 더미
├─ Data/                                   # PawnData, AbilitySet, 입력, 콤보, 타게팅 설정
├─ Characters/                             # 샘플 캐릭터 및 애니메이션 관련 에셋
├─ Maps/L_CombatTest.umap                   # 기본 전투 샘플 맵
└─ Test/                                   # 테스트용 에셋 및 맵
Config/                                    # 프로젝트 입력·맵·엔진 설정
.agents/                                   # 개발 역할 가이드 및 외부 참조 목록
```

## 핵심 클래스

| 영역 | 주요 클래스 | 역할 |
| --- | --- | --- |
| 캐릭터 구성 | `UHACF_PawnData`, `UHACF_HeroComponent` | 데이터 기반 설정, 입력 및 이동 연결 |
| 전투 입력 | `UHACF_CombatComponent`, `FHACF_InputBuffer` | 어빌리티 입력 큐, 입력 버퍼, Action Window 관리 |
| GAS 구성 | `UHACF_AbilitySet`, `UHACF_GameplayAbility` | 어빌리티·효과·AttributeSet 구성 및 어빌리티 기반 동작 |
| 콤보 | `UHACF_CombatGameplayAbility`, `UHACF_ComboDefinition`, `UHACF_AnimNotifyState_ComboWindow` | 몽타주 재생, 노드 전이, 입력 윈도우 연결 |
| 타게팅 서비스 | `IHACF_TargetingService`, `UHACF_TargetingComponent` | 타겟 요청 인터페이스와 기본 컴포넌트 구현 |
| 타게팅 실행 | `UHACF_TargetingExecutor`, `UHACF_TargetingConfig` | 설정에 따른 후보 수집·필터링·평가 및 결과 생성 |
| 속성·데미지 | `UHACF_AttributeSet`, `UHACF_DamageExecution` | 속성 범위 처리, 피해·회복 적용, 데미지 계산 |

`UHACF_TargetingService`는 Unreal 리플렉션을 위한 인터페이스 선언 클래스이며, C++ 서비스 계약은 `IHACF_TargetingService`에 정의되어 있습니다. 기본 구현인 `UHACF_TargetingComponent`는 요청을 `UHACF_TargetingExecutor`에 전달합니다.

## 캐릭터에 연결하기

샘플의 플레이어 구성을 기준으로 다음 요소를 연결합니다. 에셋을 새로 구성할 때는 `/Game/Characters/Hero/BP_Hero`와 `/Game/Data/CombatPawnData`를 함께 참고하세요.

1. Pawn에 `UHACF_HeroComponent`와 `UHACF_CombatComponent`를 추가하고, HeroComponent의 `PawnData` 속성에 사용할 `UHACF_PawnData` 에셋을 지정합니다.
2. PawnData에 `AbilitySets`, `InputConfig`, `DefaultInputMappings`를 설정합니다. AbilitySet에는 사용할 어빌리티와 초기 속성용 Gameplay Effect 등을 구성하고, InputConfig에는 입력 액션과 태그를 연결합니다. HeroComponent는 초기화 과정에서 권한이 있는 ASC에 AbilitySet을 부여합니다.
3. 플레이어의 PlayerState가 `IAbilitySystemInterface`를 구현하고 ASC를 반환하도록 구성합니다. 샘플의 `ASamplePlayerState`는 ASC와 `UHACF_AttributeSet`을 생성합니다. CombatComponent는 초기화 시 ASC의 Owner를 PlayerState, Avatar를 현재 Pawn으로 연결합니다. NPC처럼 PlayerState 대신 Pawn에 ASC를 두는 구성도 지원하며, 이 경우 Pawn이 해당 인터페이스를 구현해야 합니다.
4. GameMode의 PlayerState와 PlayerController 클래스를 구성합니다. `ASamplePlayerController`처럼 `PostProcessInput`에서 Pawn의 CombatComponent에 `ProcessAbilityInput`을 호출해야 큐에 쌓인 어빌리티 입력이 처리됩니다.
5. 플레이어 입력에는 Enhanced Input을 사용합니다. HeroComponent가 어빌리티 입력을 연결하며, 이동·시점 입력은 `ASampleCharacter::SetupPlayerInputComponent`처럼 별도로 바인딩합니다. Mover 이동과 몽타주를 사용할 때는 Pawn의 Mover 구성도 함께 준비합니다.
6. 타게팅을 사용한다면 같은 Pawn에 `UHACF_TargetingComponent` 또는 `IHACF_TargetingService`를 구현한 컴포넌트를 추가합니다. CombatComponent는 BeginPlay에서 서비스를 탐색하며, 런타임에 구성을 바꾸었다면 `RefreshTargetingService`로 다시 탐색합니다.

연결 후에는 PIE에서 초기화 로그를 확인하고, 아래 테스트 중 변경한 영역에 해당하는 그룹을 실행하세요.

## 자동화 테스트

에디터의 **Session Frontend → Automation**에서 `HyperActionCombat.Tests`를 검색해 실행합니다.

| 검증 대상 | 테스트 그룹 |
| --- | --- |
| 기본 동작 | `HyperActionCombat.Tests.Sanity` |
| 입력 시뮬레이션 | `HyperActionCombat.Tests.InputSimulation` |
| 어빌리티 입력 큐 | `HyperActionCombat.Tests.CombatComponent.InputQueue` |
| 입력 버퍼·Action Window | `HyperActionCombat.Tests.InputBuffer` |
| 초기화 상태 | `HyperActionCombat.Tests.InitState` |
| 콤보 라우팅·Notify | `HyperActionCombat.Tests.Combo` |
| 타게팅 | `HyperActionCombat.Tests.Targeting` |
| 속성 | `HyperActionCombat.Tests.AttributeSet` |
| 데미지 계산 | `HyperActionCombat.Tests.DamageCalculation` |
| 전투 통합 기반 | `HyperActionCombat.Tests.CombatVerticalSlice.Foundation` |

### 테스트 의존성과 검증 범위

테스트 모듈은 플러그인 안에 있지만, 일부 테스트는 샘플 프로젝트의 `/Game` 콘텐츠에 의존합니다. 예를 들어 `CombatVerticalSlice.Foundation`은 `/Game/Test/Maps/L_CQ_CombatVerticalSlice` 맵과 `/Game/Data/CombatPawnData` 에셋을 로드합니다. 플러그인만 다른 프로젝트로 옮기면 전체 테스트를 그대로 실행할 수 없으므로, 필요한 샘플·테스트 에셋과 그 의존성을 함께 준비하거나 테스트의 에셋 경로와 구성을 새 프로젝트에 맞게 조정해야 합니다.

`CombatVerticalSlice.Foundation`은 플레이어의 ASC·컴포넌트 초기화, 초기 속성, 기본 공격 입력과 어빌리티 활성화 계약을 확인합니다. 실제 전투 흐름에서의 피해 적용, 사망, 회피 쿨다운까지 검증하는 통합 테스트는 아닙니다. 속성과 데미지 계산 자체는 각각 `AttributeSet`, `DamageCalculation` 그룹에서 별도로 검증합니다.

### 명령줄 실행

빌드 후 명령줄에서도 전체 그룹을 실행할 수 있습니다. 아래 예시는 저장소 루트에서 실행하며, 결과는 `Saved/AutomationReports`에 출력합니다.

```powershell
$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
$ProjectPath = Join-Path (Get-Location).Path 'HACF_Sample.uproject'
$ReportPath = Join-Path (Get-Location).Path 'Saved\AutomationReports'

& "$EngineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" $ProjectPath -unattended -nop4 -nosplash '-ExecCmds=Automation RunTests HyperActionCombat.Tests' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$ReportPath" -log
```

특정 그룹만 확인하려면 `Automation RunTests` 뒤의 이름을 해당 그룹으로 변경하세요. 종료 후 로그와 보고서에서 실패 여부를 확인합니다.

## 개발 가이드

- 재사용 가능한 런타임 기능은 플러그인에 유지하고, 샘플 전용 연결만 `HACF_Sample`에 구현합니다.
- API를 수정할 때는 public 헤더와 대응하는 private 구현을 함께 확인합니다.
- 게임플레이, 입력, 어빌리티 활성화, 속성 또는 데미지 변경에는 관련 CQTest를 추가하거나 갱신합니다.
- 헤더/API 및 모듈 의존성을 변경하면 Editor 타깃을 빌드합니다.
- 입력 매핑이나 태그를 변경하면 `InputSimulation` 테스트와 `Config/DefaultInput.ini`, `HACF_Tags.ini`를 함께 확인합니다.

