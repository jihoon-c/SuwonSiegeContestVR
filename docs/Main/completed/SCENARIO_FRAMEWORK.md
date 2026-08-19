# 작업

VR 교육용 `Interaction → Scene → Scenario` Core 프레임워크 구현

# 구현 내용

- ID 기반 Interaction 분기와 Scenario/Scene/Interaction 상태 관리
- Primary Data Asset 기반 Scenario/Scene 정의와 참조 유효성 검사
- Actor 사건 보고용 `UScenarioInteractableComponent`
- HMD 시야각·거리·연속 응시·가시선 기반 `UScenarioObservationComponent`
- 기존 나레이션 시스템 재사용용 `UScenarioNarrationBridgeComponent`
- 배치 가능한 `BP_ScenarioManager`
- 재시작, Skip, 강제 완료, Scene/Interaction 점프, 상태 로그 API
- 데이터 검증 및 동기 진행 자동화 테스트

# 변경 파일

- `Source/SuwonSiegeContestVR/Public/Core/Scenario/*`
- `Source/SuwonSiegeContestVR/Private/Core/Scenario/*`
- `Source/SuwonSiegeContestVR/Private/Tests/ScenarioFrameworkTests.cpp`
- `Content/Core/Scenario/Managers/BP_ScenarioManager.uasset`
- `Scripts/CreateScenarioAssets.py`
- `docs/Main/specs/SCENARIO_SYSTEM.md`

# 주요 결정 사항

- 새 Narration Manager는 만들지 않고 기존 `UNarrationSequenceComponent`를 Bridge로 연결했다.
- Scenario Manager는 Level 내부 흐름만 담당한다. Level Travel과 영속 진행도는 향후 `ExperienceSubsystem` 책임이다.
- Core는 특정 Game Feature를 참조하지 않는다. 신기전 등 Feature Actor는 ID 기반 사건만 보고한다.
- 디버그 `Skip`은 필수 Interaction도 완료 조건을 만족한 것으로 취급한다.

# 테스트 결과

- `SuwonSiegeContestVR Win64 Development`: 성공
- `SuwonSiegeContestVREditor Win64 Development`: 성공
- `CompileAllBlueprints`: 0 errors, 0 warnings, 0 failed loads
- `SuwonSiegeContestVR.Core.Scenario` 자동화 테스트 2개: 성공
  - 잘못된 Interaction 참조 검출
  - Objective 즉시 완료 → Wait 즉시 완료 → Scene/Scenario 종료

# 남은 문제

- 실제 교육 Scenario/Scene Data Asset 내용은 기획 데이터가 필요하므로 생성하지 않았다.
- Objective UI, Quiz, Voice Recognition, ExperienceSubsystem은 별도 작업이다.
- Android SDK가 현재 UE 검증에서 `INVALID r27c`로 표시되므로 Android 실기/패키징 검증은 환경 설정 후 필요하다.

## 2026-08-13 연결 보완

- 원인: `DA_Scene_Singijeon`은 `ScenarioSceneData`여서 `ScenarioDefinition` 슬롯에 직접 지정할 수 없었다.
- `DA_Scenario_Singijeon`을 생성하고 `DA_Scene_Singijeon`을 Scenes 배열에 연결했다.
- `AScenarioManagerActor` 최상위 Details에 Scenario Definition, Narration Table, Auto Start를 노출했다.
- `LV_Singijeon`의 Manager에 위 세 설정을 저장했다.
- Scene/Scenario/Interaction ID와 Narration Row 참조, Level Manager 참조를 자동 검사해 모두 통과했다.
