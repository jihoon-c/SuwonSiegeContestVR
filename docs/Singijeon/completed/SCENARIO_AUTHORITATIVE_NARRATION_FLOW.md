# 작업

`DA_Scene_Singijeon` 중심 나레이션·상호작용 흐름 정리

> 2026-08-18 후속 마이그레이션으로 이 데이터는 `DA_Scenario_Singijeon.Stages[Singijeon]`에 인라인 저장되며 기존 Scene Asset은 삭제됐다.

# 구현 내용

- `DT_Narration`의 `NA_01~NA_21`에서 `NextRow`를 제거하고 `AdvanceMode=Stop`으로 통일했다.
- `DA_Scene_Singijeon`에 `NAR_01~NAR_21` Narration Interaction을 추가했다.
- 대사의 의미에 맞춰 기존 `INT_01~INT_07` 전후에 나레이션을 배치했다.
- 기존 Gameplay Interaction의 Type, TargetID 및 실패 재시도 계약을 유지했다.
- 이동된 신기전 Level 경로 `/Game/Maps/LV_Singijeon`을 테스트와 설정/검증 스크립트에 반영했다.

# 변경 파일

```text
Content/Data/DA_Scene_Singijeon.uasset
Content/Data/DT_Narration.uasset
Scripts/ConfigureSingijeonScenarioFlow.py
Scripts/VerifySingijeonScenarioFlow.py
Source/SuwonSiegeContestVR/Private/Tests/ScenarioFrameworkTests.cpp
관련 Level 설정/검증 스크립트 및 문서
```

# 주요 결정 사항

- 체험 전체 순서는 `DA_Scene_*`만 소유한다.
- `DT_Narration.NextRow`는 Scenario와 무관한 독립 대사 묶음에서만 사용한다.
- 신기전에서는 각 나레이션 Row를 하나의 Scenario Interaction으로 실행한다.

# 테스트 결과

- DA Scene 28개 Interaction 전체 연결 재조회 성공
- DT의 21개 Row 모두 `NextRow=None`, `AdvanceMode=Stop` 확인
- 이동된 Level과 Scenario Manager 참조 확인
- Win64 Development Editor 빌드 성공
- Core 자동화 테스트 5개 성공

# 남은 문제

- `INT_03 / Hwacha_Aim`, `INT_05 / Torch_Ignite`의 실제 완료 보고자는 별도 Feature 구현이 필요하다.
- 실제 음성 타이밍과 행동 안내 간격은 VR Preview에서 체감 검증이 필요하다.
