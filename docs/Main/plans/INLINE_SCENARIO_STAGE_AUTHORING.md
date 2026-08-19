# 목적

레벨마다 `Experience`, `Scenario`, `Scene`을 각각 중복 연결하던 제작 구조를 `Experience → Scenario → Stage(인라인) → Interaction`으로 단순화한다.

**상태: 완료 (2026-08-18)**

# 현재 상태

- Level Manager에 Experience와 Scenario를 동시에 지정할 수 있어 불일치가 발생할 수 있다.
- `DA_Scenario_*`를 열어도 실제 Interaction은 별도 `DA_Scene_*`를 열어야 확인할 수 있다.
- 대부분의 체험은 `1 Level = 1 Scenario = 1 Scene`이라 Scene Data Asset이 불필요한 래퍼가 된다.

# 구현 범위

- Scenario Definition 내부에 Stage와 Interaction을 인라인 저장
- Experience Definition이 Scenario Definition과 자동 시작/완료 정책 소유
- Level Manager는 Experience Definition 하나로 Scenario와 Narration Table 자동 해석
- Main/신기전 기존 Scene Data를 Stage로 마이그레이션
- 기존 Scene 기반 C++ API는 호환용으로 유지하되 신규 제작 경로에서는 숨김

# 변경 예정 파일

```text
Source/SuwonSiegeContestVR/*/Core/Scenario/*
Source/SuwonSiegeContestVR/*/Core/Experience/*
Content/Data/DA_Scenario_Main.uasset
Content/Data/DA_Scenario_Singijeon.uasset
Content/Core/Experience/Definitions/DA_Experience_*.uasset
Content/Maps/Main/L_Main.umap
Content/Maps/LV_Singijeon.umap
Scripts/MigrateScenarioScenesToInlineStages.py
Scripts/VerifyInlineScenarioStages.py
docs/Main/specs/SCENARIO_SYSTEM.md
```

# 구현 단계

1. 인라인 Stage 구조 및 하위 호환 해석 추가
2. Experience 중심 Manager 자동 설정 추가
3. Main/신기전 에셋 마이그레이션
4. 기존 별도 Scene Data Asset 참조 제거
5. 빌드, 에셋 재조회, Core 자동화 테스트

# 구현 결과

- Scenario Definition에서 Stage와 모든 Interaction을 한 화면에 편집
- Scenario가 Narration Table을 소유
- Experience가 Scenario와 자동 시작/완료 정책을 소유
- Level Manager는 Experience 하나만 편집하고 나머지는 읽기 전용으로 자동 해석
- Main/신기전 별도 Scene Data Asset 제거

# 다른 Feature에 미치는 영향

- Core 런타임만 변경하며 특정 Game Feature를 직접 참조하지 않는다.
- 기존 `UScenarioSceneData`와 Scene 명칭 API는 하위 호환을 위해 유지한다.
- 아직 Scenario를 만들지 않은 다른 체험은 신규 Stage 방식만 사용하면 된다.

# 검증 방법

- Win64 Development Editor 빌드
- Main/신기전 Scenario의 Stage/Interaction 인라인 데이터 재조회
- Level Manager가 Experience 하나로 Scenario/Narration을 해석하는지 확인
- `SuwonSiegeContestVR.Core` 자동화 테스트
