# 상태

`Completed (2026-08-13)` — 결과는 `docs/Main/completed/SCENARIO_FRAMEWORK.md` 참고.

# 목적

교육 콘텐츠를 `Interaction → Scene → Scenario` 계층으로 구성하는 Core C++ 프레임워크를 구현한다. 콘텐츠 데이터와 연출은 Blueprint/Data Asset에서 제작하고, Actor는 ID 기반 사건만 보고하도록 결합도를 제한한다.

# 현재 상태

- `UNarrationSequenceComponent`가 Data Table 기반 음성·자막 재생과 행 내부 흐름을 담당한다.
- `ExperienceSubsystem`은 미구현이며 향후 레벨 전환과 레벨 간 진행도를 담당할 예정이다.
- 신기전은 장전·점화·발사·양손 운반 기능은 있으나 체험 진행 Manager가 없다.
- Scenario/Scene/Interaction 공통 데이터와 상태 관리 시스템은 없다.

# 구현 범위

- Scenario/Scene/Interaction 데이터 타입과 상태 Enum
- `UScenarioDefinition`, `UScenarioSceneData` Primary Data Asset
- `UScenarioManagerComponent`의 시작·완료·실패·분기·재시작·점프·디버그 API
- `UScenarioInteractableComponent`의 ID 기반 시작/진행/완료/실패 보고
- `UScenarioObservationComponent`의 시야각·거리·가시선·유지 시간 판정
- 기존 나레이션 시스템과의 Blueprint 연결 계약 및 사용 문서

# 변경 예정 파일

- `Source/SuwonSiegeContestVR/Public/Core/Scenario/*`
- `Source/SuwonSiegeContestVR/Private/Core/Scenario/*`
- `docs/ARCHITECTURE.md`
- `docs/DIRECTORY_STRUCTURE.md`
- `docs/Main/STATUS.md`
- `docs/Main/specs/SCENARIO_SYSTEM.md`
- `docs/Main/completed/SCENARIO_FRAMEWORK.md`

# 구현 단계

1. 기존 Narration/Experience 책임과 중복되지 않는 경계를 확정한다.
2. 데이터 타입과 Primary Data Asset을 구현한다.
3. 이벤트 기반 Scenario Manager와 분기/상태 검증을 구현한다.
4. Actor 보고용 Interactable과 관찰용 Observation Component를 구현한다.
5. C++ 빌드와 Blueprint 전체 컴파일을 검증한다.
6. 콘텐츠 제작 및 기존 나레이션 연결 방법을 문서화한다.

# 다른 Feature에 미치는 영향

- Core에만 구현하며 특정 Game Feature를 참조하지 않는다.
- 각 Game Feature는 Core Scenario Data/Component를 참조할 수 있다.
- `ExperienceSubsystem`의 레벨 전환·영속 진행도 책임은 침범하지 않는다.
- 기존 `UNarrationSequenceComponent`를 유지하여 나레이션 기능을 중복 구현하지 않는다.

# 검증 방법

- `SuwonSiegeContestVR Win64 Development` 빌드
- `SuwonSiegeContestVREditor Win64 Development` 빌드
- `CompileAllBlueprints` 오류·경고·로드 실패 확인
- 데이터 참조·중복 ID·분기 대상의 런타임 검증 로그 확인
