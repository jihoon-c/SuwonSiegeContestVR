# 목적

**상태: 완료 (2026-08-21)**

에디터와 Development 빌드에서 Spacebar로 현재 시나리오 인터랙션을 완료하고 다음 인터랙션으로 진행한다.

# 현재 상태

`ScenarioManagerComponent`에는 `CompleteCurrentInteraction()` 디버그 API가 있지만 키 입력 연결과 나레이션 정리 처리가 없다.

# 구현 범위

- `ScenarioManagerActor`의 Spacebar 디버그 입력
- 현재 나레이션 중지 및 지연 완료 콜백 제거
- Shipping 빌드 비활성화
- Core 자동화 테스트

# 변경 예정 파일

- `ScenarioManagerActor.h/.cpp`
- `ScenarioNarrationBridgeComponent.h/.cpp`
- `ScenarioFrameworkTests.cpp`
- `docs/Core/`

# 구현 단계

1. Space 입력을 레벨의 Scenario Manager에 등록
2. 나레이션을 안전하게 취소하고 현재 인터랙션 성공 처리
3. 다음 인터랙션 및 마지막 인터랙션 완료 테스트

# 다른 Feature에 미치는 영향

모든 Feature의 Scenario Manager에 공통 적용된다. Shipping 빌드에서는 동작하지 않는다.

# 검증 방법

Development Editor 빌드와 Core Scenario 자동화 테스트를 실행한다.
