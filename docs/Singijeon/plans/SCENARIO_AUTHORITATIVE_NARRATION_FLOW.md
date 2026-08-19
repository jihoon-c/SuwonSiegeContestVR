# 목적

`DA_Scene_Singijeon`을 신기전 체험 순서의 단일 원본으로 만들고, `DT_Narration.NextRow` 자동 체인과의 이중 진행을 제거한다.

> 상태: 완료 (2026-08-18)

# 현재 상태

- `DT_Narration`의 `NA_01~NA_20`이 `NextRow`와 `Auto`로 `NA_21`까지 연결되어 있다.
- `DA_Scene_Singijeon`에는 `NA_01`, `NA_02`만 Interaction으로 있고 나머지 Row는 Scenario 흐름에 없다.
- 이동된 Level 경로는 `/Game/Maps/LV_Singijeon`이며 일부 테스트/스크립트는 이전 플러그인 경로를 사용한다.

# 구현 범위

- `DT_Narration`: 모든 Row의 `NextRow=None`, `AdvanceMode=Stop`
- `DA_Scene_Singijeon`: `NA_01~NA_21`을 행동 단계 사이의 Narration Interaction으로 구성
- 기존 `INT_01~INT_07`의 Type/TargetID 유지
- Level 경로를 참조하는 현재 자동화 테스트와 설정/검증 스크립트 갱신
- 에셋 재조회 및 Core 자동화 테스트

# 변경 예정 파일

```text
Content/Data/DT_Narration.uasset
Content/Data/DA_Scene_Singijeon.uasset
Scripts/ConfigureSingijeonScenarioFlow.py
Scripts/VerifySingijeonScenarioFlow.py
Source/SuwonSiegeContestVR/Private/Tests/ScenarioFrameworkTests.cpp
Level 경로를 사용하는 Scripts 및 관련 문서
```

# 구현 단계

1. 나레이션/행동 순서를 Interaction 체인으로 재구성
2. DT Row 자동 체인 제거
3. 이동된 Level 경로 참조 갱신
4. Unreal 에셋 재조회
5. 빌드 및 자동화 테스트

# 다른 Feature에 미치는 영향

- Core Scenario/Narration 런타임 코드는 변경하지 않는다.
- 다른 Feature의 DT/DA에는 영향이 없다.
- 신기전 Gameplay Actor의 `INT_01~INT_07` 보고 계약은 유지한다.

# 검증 방법

- DT의 모든 `NextRow=None`, `AdvanceMode=Stop` 확인
- DA Scene의 전체 Interaction 연결과 NarrationID 확인
- `/Game/Maps/LV_Singijeon` Manager 참조 확인
- Win64 Editor 빌드 및 `SuwonSiegeContestVR.Core` 자동화 테스트

# 검증 결과

- 21개 Narration + 7개 Gameplay Interaction 연결 재조회 성공
- DT의 모든 `NextRow=None`, `AdvanceMode=Stop` 확인
- `/Game/Maps/LV_Singijeon` 및 Scenario Manager 참조 확인
- Win64 Development Editor 빌드 성공
- `SuwonSiegeContestVR.Core` 자동화 테스트 5개 성공
