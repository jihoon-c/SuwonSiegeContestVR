# 목적

상태: 완료

각 Level에 배치된 Scenario Manager에서 해당 Level 전용 Narration Data Table을 지정할 수 있게 한다.

# 현재 상태

- Manager의 Narration Table은 Scenario Definition에서 자동 해석되는 읽기 전용 값이다.
- Level별로 동일 Scenario를 사용하면서 다른 Narration DT를 선택할 수 없다.

# 구현 범위

- Manager에 선택형 `Level Narration Table` 편집 슬롯 추가
- 값이 있으면 Level 설정을 우선하고, 비어 있으면 Scenario 기본 DT 사용
- 기존 Level과 Scenario DA의 설정 및 런타임 흐름 유지

# 변경 예정 파일

```text
Source/SuwonSiegeContestVR/Public/Core/Scenario/ScenarioManagerActor.h
Source/SuwonSiegeContestVR/Private/Core/Scenario/ScenarioManagerActor.cpp
Source/SuwonSiegeContestVR/Private/Tests/ScenarioFrameworkTests.cpp
docs/Main/specs/SCENARIO_SYSTEM.md
docs/Main/completed/LEVEL_NARRATION_TABLE_OVERRIDE.md
```

# 구현 단계

1. Level Narration Table Override 프로퍼티 추가
2. Manager 설정 해석 우선순위 적용
3. 기본값과 Override 자동화 테스트 추가
4. Editor 빌드 및 Core 회귀 테스트

# 다른 Feature에 미치는 영향

- Core Manager의 선택형 설정만 추가한다.
- Override가 비어 있는 기존 Feature는 기존 Scenario DT를 그대로 사용한다.

# 검증 방법

- Win64 Development Editor 빌드
- Override 지정/해제 시 해석 결과 자동화 테스트
- 기존 신기전 Scenario/Experience 에셋 검증

# 결과

- Manager의 `Level Narration Table`을 레벨별로 편집할 수 있다.
- 지정하면 레벨 값을 우선하고, 비워 두면 Scenario Definition의 Narration Table을 사용한다.
- Editor 빌드와 Core 자동화 테스트를 통과했다.
