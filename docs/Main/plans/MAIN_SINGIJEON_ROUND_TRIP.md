# 목적

`L_Main`의 특정 이벤트에서 신기전 Experience로 이동하고, 신기전 Scenario 완료 후 Main의 지정된 다음 단계로 복귀한다.

**상태: 완료 (2026-08-18)**

**2026-08-18 보완:** 충돌 Primitive가 없는 `BP_VRPlayerPawn`도 이동 구역에 진입할 수 있도록
Player Camera(HMD) 위치 기반 Trigger 판정을 추가한다.
신기전 Level 패키지에 남은 이전 Scenario Asset 이름 참조도 강제 재저장으로 제거한다.

# 현재 상태

- `L_Main`은 `/Game/Maps/Main/L_Main`에 존재한다.
- 배치된 `BP_ScenarioManager`가 신기전 Scenario/Experience를 참조하고 있다.
- `UExperienceSubsystem`은 Experience 완료 목록은 유지하지만 Main Scenario 복귀 체크포인트는 저장하지 않는다.
- `DA_Experience_Singijeon.ReturnLevel`은 Main 복귀용으로 확정해야 한다.

# 구현 범위

- `DA_Scenario_Main`: 인라인 Main Stage와 예시 Interaction 흐름
- `DA_Experience_Main`: Main Level Experience Definition
- `AExperienceTravelTriggerActor`: Pawn Overlap, HMD 위치 진입 또는 Blueprint Event로 Experience 이동
- `UExperienceSubsystem`: Scenario 복귀 체크포인트의 세션 메모리 보존
- `UScenarioExperienceBridgeComponent`: Main 재진입 시 체크포인트 복원
- `L_Main`: Main Manager 재설정, Trigger/PlayerStart 배치
- `DA_Experience_Singijeon`: ReturnLevel을 `L_Main`으로 설정

# 변경 예정 파일

```text
Source/SuwonSiegeContestVR/*/Core/Experience/*
Source/SuwonSiegeContestVR/*/Core/Scenario/ScenarioExperienceBridgeComponent.*
Source/SuwonSiegeContestVR/*/Core/Scenario/ScenarioManagerActor.*
Content/Data/DA_Scenario_Main.uasset
Content/Core/Experience/Definitions/DA_Experience_Main.uasset
Content/Core/Experience/Definitions/DA_Experience_Singijeon.uasset
Content/Maps/Main/L_Main.umap
Scripts/CreateMainSingijeonRoundTrip.py
Scripts/VerifyMainSingijeonRoundTrip.py
```

# 구현 단계

1. 세션 복귀 체크포인트 API와 범용 Travel Trigger 구현
2. Scenario Manager가 자동 시작 후 체크포인트를 복원하도록 연결
3. Main용 DA 3종 생성 및 예시 흐름 작성
4. Main Level Manager/Trigger/PlayerStart 구성
5. 신기전 ReturnLevel 연결
6. 빌드, 자동화 테스트, 에셋 재조회

# 구현 결과

- `DA_Scenario_Main` 인라인 Stage와 `DA_Experience_Main` 생성
- `L_Main`에 범용 `ExperienceTravelTriggerActor`와 `PlayerStart` 배치
- 신기전 완료 시 `L_Main` 복귀 및 `MAIN_RETURNED` 체크포인트 복원
- Main을 Editor/Game 기본 Map으로 지정
- 세션 체크포인트와 이전 Interaction 완료 상태 복원 구현

# 다른 Feature에 미치는 영향

- Trigger와 체크포인트는 Core의 범용 기능이며 특정 신기전 클래스를 참조하지 않는다.
- 다른 Feature도 자체 `DA_Experience_*`와 복귀 Interaction ID만 지정해 재사용할 수 있다.
- Game Feature 간 직접 의존은 추가하지 않는다.

# 검증 방법

- Win64 Development Editor 빌드
- Experience 체크포인트 자동화 테스트
- Main DA/Manager/Trigger와 신기전 ReturnLevel 재조회
- Core 자동화 회귀 테스트
