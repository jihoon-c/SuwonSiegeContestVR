# 목적

Main 진행을 `공심돈 1번 시나리오 → Main 복귀 및 다음 진행 → 신기전 → Main 복귀` 순서로 구성한다.

**상태: 완료 (2026-08-19)**

# 현재 상태

- Main↔신기전 단일 왕복과 세션 체크포인트 복원은 구현돼 있다.
- `DA_Scenario_Main`은 공심돈 단계를 포함하지 않는다.
- `/GF_Gongsimdon/Maps/LV_Gongsimdon` 레벨은 있으나 Scenario/Experience/Manager 기초 설정이 없다.
- 이동 Trigger는 현재 Scenario 단계와 무관하게 작동하므로 두 Trigger를 배치하면 순서를 우회할 수 있다.

# 구현 범위

- 공심돈용 Scenario Definition과 Experience Definition 생성
- 공심돈 레벨에 PlayerStart, Scenario Manager, 기본 Scenario 01 구성
- 공심돈 Scenario 완료 시 Main 자동 복귀
- Main Scenario를 공심돈 이동/복귀/신기전 이동/복귀 순서로 재구성
- Main에 공심돈·신기전 Travel Trigger 두 개 배치
- Travel Trigger에 선택형 Required Interaction ID 가드 추가
- 체크포인트 복원 및 전체 에셋 검증

# 변경 예정 파일

```text
Source/SuwonSiegeContestVR/*/Core/Experience/ExperienceTravelTriggerActor.*
Content/Data/DA_Scenario_Main.uasset
Content/Core/Experience/Definitions/DA_Experience_Main.uasset
Content/Core/Experience/Definitions/DA_Experience_Gongsimdon.uasset
Plugins/GameFeatures/GF_Gongsimdon/Content/Data/DA_Scenario_Gongsimdon.uasset
Plugins/GameFeatures/GF_Gongsimdon/Content/Maps/LV_Gongsimdon.umap
Content/Maps/Main/L_Main.umap
Scripts/ConfigureMainGongsimdonSingijeonSequence.py
Scripts/VerifyMainGongsimdonSingijeonSequence.py
docs/Main/specs/SCENARIO_SYSTEM.md
docs/Gongsimdon/STATUS.md
```

# 구현 단계

1. Travel Trigger 현재 Interaction 가드 추가
2. 공심돈 Scenario/Experience와 레벨 Manager 기초 설정
3. Main Scenario를 공심돈 후 신기전 순서로 구성
4. Main Trigger 두 개와 각 복귀 체크포인트 설정
5. 빌드, 자동화 테스트, 에셋/레벨 재조회

# 다른 Feature에 미치는 영향

- Travel Trigger 가드는 비워 두면 기존 동작을 유지한다.
- 신기전 Scenario 내용은 변경하지 않고 ReturnLevel만 기존 Main을 유지한다.
- 공심돈은 콘텐츠 기초 Scenario만 추가하며 적/탐지 게임플레이는 후속 확장으로 남긴다.

# 검증 방법

- Win64 Development Editor 빌드
- Required Interaction ID 가드 자동화 테스트
- Main Interaction 순서와 두 Trigger 체크포인트 재조회
- 공심돈 Scenario/Experience/Manager/PlayerStart 재조회
- 공심돈·신기전 ReturnLevel 확인
- 전체 `SuwonSiegeContestVR` 자동화 테스트

# 구현 결과

- 공심돈 Scenario/Experience와 레벨 Manager/PlayerStart 기초 설정 완료
- Main에 공심돈·신기전 순차 Trigger 및 각 복귀 체크포인트 설정 완료
- `RequiredInteractionID`로 현재 Interaction과 일치하는 Trigger만 동작
- Development Editor 빌드 성공
- 에셋/레벨 구성 검증 성공
- 전체 자동화 테스트 7/7 성공
