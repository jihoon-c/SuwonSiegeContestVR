# 작업

Scenario Interaction 비파괴 사전 허용 검사 추가

# 구현 내용

- `ScenarioManagerComponent.CanReportInteractionResult`를 추가했다.
- `ScenarioInteractableComponent.CanReportInteraction`을 추가했다.
- 완료/실패 이벤트는 현재 Target/Type을 Manager가 승인한 뒤에만 Broadcast한다.
- 사전 검사는 Interaction 상태를 변경하지 않는다.

# 변경 파일

```text
Source/SuwonSiegeContestVR/*/Core/Scenario/ScenarioManagerComponent.*
Source/SuwonSiegeContestVR/*/Core/Scenario/ScenarioInteractableComponent.*
Source/SuwonSiegeContestVR/Private/Tests/ScenarioFrameworkTests.cpp
```

# 주요 결정 사항

- Manager가 없는 독립 테스트/샌드박스에서는 기존과 같이 유효한 Interactable 보고를 허용한다.
- 활성 Manager가 있으면 Running Interaction의 Target/Type이 정확히 일치해야 한다.
- `docs/ARCHITECTURE.md`는 수정하지 않았다.

# 테스트 결과

- Win64 Development Editor 빌드 성공
- 현재 Target/Type 허용, 미래 Target/잘못된 Type 거절, 사전 검사 무부작용 자동화 성공
- 프로젝트 자동화 16개 전체 성공

# 남은 문제

- 없음
