# 작업

Scenario Interactor의 가이드 앵커를 레벨 뷰포트에서 직접 이동하고 저장할 수 있게 했다.

# 구현 내용

- `ScenarioInteractableComponent`용 Component Visualizer를 Editor 모듈에 등록했다.
- 액터 선택 후 노란 앵커를 클릭하면 이동 기즈모가 표시된다.
- 이동 결과는 해당 컴포넌트의 `Guide Anchor Offset`에 저장된다.
- Undo/Redo와 Details 패널 갱신을 지원한다.
- 에디터 화살표 자체는 대상 Bounds 계산에서 제외해 이동 중 기준점이 바뀌지 않는다.

# 변경 파일

- `ScenarioInteractableComponent.h/.cpp`
- `ScenarioGuideComponentVisualizer.h/.cpp`
- `SuwonSiegeContestVREditor.h/.cpp`
- `ScenarioFrameworkTests.cpp`

# 주요 결정 사항

런타임용 Scene Component를 추가하지 않고 Editor 전용 Visualizer를 사용한다. 따라서 저장되는 데이터는 기존 `Guide Anchor Offset` 하나이며 패키징 런타임 비용은 없다.

# 테스트 결과

- Editor 빌드 성공
- Core Scenario 자동화 테스트 6/6 성공
- 월드 위치 Setter가 Offset에 저장되고 동일 위치를 반환하는 테스트 성공

# 남은 문제

- 없음
