# 작업

Spacebar 시나리오 인터랙션 디버그 진행 기능

# 구현 내용

- 레벨의 `AScenarioManagerActor`가 Spacebar 입력을 받으면 현재 Running 인터랙션을 성공 완료한다.
- 성공 분기 또는 `NextInteractionID`를 따라 다음 인터랙션으로 진행한다.
- 나레이션 중 사용하면 재생과 지연 콜백을 먼저 취소해 다음 나레이션과 겹치지 않게 한다.
- Shipping 빌드에서는 입력과 API가 동작하지 않는다.
- 에디터에서 `Enable Spacebar Debug Advance`로 기능을 켜고 끌 수 있다.

# 변경 파일

- `ScenarioManagerActor.h/.cpp`
- `ScenarioNarrationBridgeComponent.h/.cpp`
- `ScenarioFrameworkTests.cpp`
- `docs/Core/plans/SCENARIO_SPACEBAR_DEBUG_ADVANCE.md`

# 주요 결정 사항

Pawn이나 Feature Blueprint가 아닌 레벨 공통 Scenario Manager가 입력을 소유한다. 따라서 모든 Feature에서 같은 방식으로 동작하며 Feature가 Core 입력을 중복 구현하지 않는다.

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- 첫 인터랙션에서 다음 인터랙션으로 진행 성공
- 마지막 인터랙션에서 Scenario 완료 성공
- 전체 자동화 테스트 15/15 성공

# 남은 문제

Spacebar는 에디터 창 또는 PIE/VR Preview 창에 키보드 포커스가 있을 때만 입력된다.
