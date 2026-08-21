# 작업

Main에서 공심돈 Scenario 01을 먼저 진행하고, Main 복귀 후 이전 진행의 다음 단계부터 신기전으로 이동하는 순차 Experience 흐름을 구현했다.

# 구현 내용

- Main Interaction 순서를 아래와 같이 구성했다.

```text
MAIN_INTRO
  → MAIN_TRAVEL_GONGSIMDON
  → LV_Gongsimdon
  → MAIN_AFTER_GONGSIMDON
  → MAIN_TRAVEL_SINGIJEON
  → LV_Singijeon
  → MAIN_AFTER_SINGIJEON
```

- 공심돈용 `DA_Scenario_Gongsimdon`, `DA_Experience_Gongsimdon`을 생성했다.
- 공심돈 레벨에 Scenario Manager와 PlayerStart를 설정했다.
- 공심돈 Scenario 01은 야간 경계 Action 13단계를 완료하면 Main으로 자동 복귀한다.
- Main에 공심돈·신기전 Travel Trigger를 배치하고 각 복귀 Interaction을 체크포인트로 저장한다.
- Travel Trigger에 `RequiredInteractionID`를 추가해 두 Trigger가 현재 Main 단계와 일치할 때만 이동하게 했다.
- 공심돈 복귀 시 `MAIN_AFTER_GONGSIMDON`, 신기전 복귀 시 `MAIN_AFTER_SINGIJEON`부터 재개한다.

# 변경 파일

```text
Source/SuwonSiegeContestVR/Public/Core/Experience/ExperienceTravelTriggerActor.h
Source/SuwonSiegeContestVR/Private/Core/Experience/ExperienceTravelTriggerActor.cpp
Source/SuwonSiegeContestVR/Private/Tests/ScenarioFrameworkTests.cpp
Content/Data/DA_Scenario_Main.uasset
Content/Core/Experience/Definitions/DA_Experience_Gongsimdon.uasset
Content/Maps/Main/L_Main.umap
Plugins/GameFeatures/GF_Gongsimdon/Content/Data/DA_Scenario_Gongsimdon.uasset
Plugins/GameFeatures/GF_Gongsimdon/Content/Maps/LV_Gongsimdon.umap
Scripts/ConfigureMainGongsimdonSingijeonSequence.py
Scripts/VerifyMainGongsimdonSingijeonSequence.py
Scripts/VerifyMainSingijeonRoundTrip.py
docs/Main/specs/SCENARIO_SYSTEM.md
docs/Main/STATUS.md
docs/Gongsimdon/STATUS.md
```

# 주요 결정 사항

- 진행 복원은 기존 `UExperienceSubsystem` 세션 체크포인트를 재사용한다.
- Core가 공심돈 Feature 클래스를 직접 참조하지 않으며 Data Asset으로 연결한다.
- `RequiredInteractionID`가 비어 있으면 기존 Trigger 동작을 유지한다.
- 공심돈 Scenario는 Feature 전용 Director가 Core Scenario 요청을 관찰·보고·사격 판정 Actor로 전달한다.

# 테스트 결과

- Win64 Development Editor 빌드: 성공
- `VerifyMainGongsimdonSingijeonSequence.py`: 전 항목 성공
- `VerifyMainSingijeonRoundTrip.py`: 성공
- `Automation RunTests SuwonSiegeContestVR`: 7/7 성공

# 남은 문제

- 공심돈의 실제 음향·적·봉돈 연출과 보고 UI, 무기 입력을 판정 Actor에 연결해야 한다.
- Quest 3 또는 PC VR PIE에서 전체 이동 동선을 수동 확인해야 한다.
- 진행은 현재 세션 메모리 기반이라 앱 재시작 후에는 유지되지 않는다.
