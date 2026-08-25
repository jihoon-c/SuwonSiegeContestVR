# 작업

Scenario Interactor별 런타임 가이드 위치를 실행 전 에디터에서 확인하고 조절할 수 있게 했다.

# 구현 내용

- 모든 `ScenarioInteractableComponent`가 Actor Bounds 상단에 노란 에디터 전용 Arrow를 표시한다.
- `Guide Anchor Offset`으로 대상별 위치를 조절한다.
- 기본 Z Offset을 기존 35cm에서 10cm로 낮췄다.
- 런타임 Widget이 Manager 공통 높이 대신 선택된 Interactor의 Anchor 위치를 사용한다.
- `Show Guide Anchor In Editor`로 편집 마커 표시만 끌 수 있다.

# 변경 파일

- `ScenarioInteractableComponent.h/.cpp`
- `ScenarioInteractionGuideComponent.h/.cpp`
- `ScenarioFrameworkTests.cpp`
- `docs/Main/specs/SCENARIO_SYSTEM.md`

# 주요 결정 사항

가이드 Widget 자체는 기존처럼 활성 Interaction 하나만 런타임 생성한다. 에디터에서는 가벼운 Arrow만 표시해 UMG 생성과 Tick 비용을 만들지 않는다.

# 테스트 결과

- Editor 빌드 성공
- Core Scenario 자동화 테스트 6/6 성공
- `LV_Singijeon`의 모든 Scenario Interactor에 편집용 Anchor 생성 확인

# 남은 문제

- 각 대상의 최종 Offset은 레벨 아트 배치에 맞춰 에디터에서 육안 조정할 수 있다.
