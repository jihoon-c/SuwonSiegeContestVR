# 작업

Scenario 제작 구조 단순화 및 인라인 Stage 마이그레이션

# 구현 내용

- Interaction 배열 항목에 `InteractionID` 제목을 표시하고, Narration/Objective/Wait/실제 Interaction 타입별로 필요한 옵션만 보이도록 Details 조건부 표시를 적용했다.

- `UScenarioDefinition.Stages` 안에서 Stage와 Interaction 전체 흐름을 한 번에 편집하도록 변경했다.
- `UScenarioDefinition`이 Narration Table을 소유하도록 이동했다.
- `UExperienceDefinition`이 Scenario Definition, 자동 시작, Scenario 종료 시 Experience 완료 정책을 소유한다.
- Level의 `BP_ScenarioManager`에는 Experience Definition 하나만 지정하며 Scenario/Narration/정책은 읽기 전용으로 자동 해석한다.
- Main과 신기전 데이터를 인라인 Stage로 마이그레이션했다.
- 중복이던 `DA_Main`, `DA_Scene_Singijeon` 에셋은 참조 제거 검증 후 삭제했다.
- 기존 Scene 기반 C++ 런타임 API와 데이터 해석은 하위 호환용으로 유지했다.

# 변경 파일

```text
Source/SuwonSiegeContestVR/*/Core/Scenario/*
Source/SuwonSiegeContestVR/*/Core/Experience/ExperienceDefinition.*
Content/Data/DA_Scenario_Main.uasset
Content/Data/DA_Scenario_Singijeon.uasset
Content/Core/Experience/Definitions/DA_Experience_Main.uasset
Content/Core/Experience/Definitions/DA_Experience_Singijeon.uasset
Content/Maps/Main/L_Main.umap
Content/Maps/LV_Singijeon.umap
Scripts/MigrateScenarioScenesToInlineStages.py
Scripts/VerifyInlineScenarioStages.py
```

# 주요 결정 사항

- 제작 계층은 `Experience → Scenario → Stage → Interaction`이다.
- Stage는 재사용용 독립 Asset이 아니라 Scenario 내부 편집 단위다.
- Level과 Scenario의 중복 연결을 막기 위해 Experience를 유일한 Level Manager 입력으로 사용한다.
- 런타임 체크포인트의 기존 Scene 명칭은 직렬화/Blueprint 호환을 위해 내부적으로 유지하되 에디터에는 Stage ID로 표시한다.

# 테스트 결과

- Win64 Development Editor 빌드 성공
- Main/신기전 인라인 Stage 및 단일 Manager 설정 재조회 성공
- 중복 Scene Data Asset 제거 후 재검증 성공
- `SuwonSiegeContestVR.Core` 자동화 테스트 5개 성공

# 남은 문제

- 기존 문서나 외부 Blueprint에서 Scene 명칭 API를 호출해도 동작하지만 신규 작업에서는 Stage 용어를 사용해야 한다.
- 실제 Quest PIE에서 Main↔신기전 전체 왕복을 최종 확인해야 한다.
