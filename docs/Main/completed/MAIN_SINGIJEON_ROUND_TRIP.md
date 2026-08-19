# 작업

Main → 신기전 → Main 왕복과 Main Scenario 진행상태 복원

# 구현 내용

- `DA_Scenario_Main.Stages[MAIN_SCENE]`에 `MAIN_INTRO → MAIN_TRAVEL_SINGIJEON` 예시 흐름과 복귀 지점 `MAIN_RETURNED`를 구성했다.
- `DA_Scenario_Main`, `DA_Experience_Main`을 생성하고 `L_Main`의 `BP_ScenarioManager`에 연결했다.
- `L_Main`에 Pawn Overlap, HMD 위치 진입 또는 Blueprint 호출로 쓸 수 있는 `ExperienceTravelTriggerActor`를 배치했다.
- 충돌 Primitive가 없는 `BP_VRPlayerPawn`에서는 Player Camera(HMD) 위치를 Trigger Box와 비교해 이동시킨다.
- `LV_Singijeon`에 남아 있던 삭제된 `DA_Scenario_SuwonSiege` 패키지 참조를 강제 재저장으로 제거했다.
- 이동 직전에 Main Scenario/Scene/Interaction 체크포인트를 `UExperienceSubsystem`에 저장한다.
- 신기전 Scenario 종료 시 `DA_Experience_Singijeon.ReturnLevel`을 통해 `L_Main`으로 복귀한다.
- Main 재진입 시 체크포인트 이전 Interaction을 완료 상태로 복원하고 `MAIN_RETURNED`부터 진행한다.
- `EditorStartupMap`, `GameDefaultMap`을 `L_Main`으로 변경했다.

# 변경 파일

```text
Source/SuwonSiegeContestVR/*/Core/Experience/*
Source/SuwonSiegeContestVR/*/Core/Scenario/*
Content/Data/DA_Scenario_Main.uasset
Content/Core/Experience/Definitions/DA_Experience_Main.uasset
Content/Core/Experience/Definitions/DA_Experience_Singijeon.uasset
Content/Maps/Main/L_Main.umap
Config/DefaultEngine.ini
Scripts/CreateMainSingijeonRoundTrip.py
Scripts/VerifyMainSingijeonRoundTrip.py
```

# 주요 결정 사항

- Level 이동은 계속 `UExperienceSubsystem`이 소유하고 Scenario는 Level 내부 흐름만 관리한다.
- 복귀 진행도는 GameInstance 수명의 세션 메모리이며 앱 재실행 영속화는 하지 않는다.
- Trigger는 특정 Feature 클래스를 참조하지 않고 `UExperienceDefinition`과 ID만 사용한다.
- Pawn Overlap과 HMD 위치 판정을 모두 지원하며, 다른 Blueprint 이벤트에서도 `TriggerExperienceTravel`을 호출할 수 있다.

# 테스트 결과

- Win64 Development Editor 빌드 성공
- Main 왕복 에셋/Level 재조회 검증 성공
- `SuwonSiegeContestVR.Core` 자동화 테스트 5개 성공

# 남은 문제

- 실제 HMD PIE 왕복 플레이 검증은 에디터에서 수행해야 한다.
- 앱 재실행 후에도 진행도를 유지하려면 `SaveGame` 영속화가 필요하다.
- VR Level 전환 페이드/로딩 화면은 별도 작업이다.
