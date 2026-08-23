# 목적

Gameplay 상태를 바꾸기 전에 현재 Scenario Interaction이 해당 Target/Type 보고를 받을 수 있는지 비파괴 방식으로 확인한다.

**상태: 완료 (2026-08-21)**

# 현재 상태

- `ReportInteractionResult`만 현재 단계 일치를 검사하며 호출 즉시 완료/실패 상태를 변경한다.
- `ScenarioInteractableComponent`는 Manager가 거절하기 전에 로컬 완료 이벤트를 먼저 Broadcast한다.

# 구현 범위

- Scenario Manager에 비파괴 `CanReportInteractionResult` 추가
- Scenario Interactable에 `CanReportInteraction` 추가
- 완료/실패 로컬 이벤트는 Manager 승인 후에만 Broadcast

# 변경 예정 파일

- Core Scenario Manager/Interactable C++ 및 자동화 테스트
- `docs/Core/`

# 구현 단계

1. 현재 Running Interaction의 Target/Type 검사 API 추가
2. 기존 Report 경로가 동일 검사를 재사용하도록 정리
3. 승인·거절 및 부작용 없음 자동화 검증

# 다른 Feature에 미치는 영향

기존 정상 순서의 보고 결과는 동일하다. 순서가 맞지 않는 보고의 로컬 완료 이벤트만 더 이상 발생하지 않는다. `docs/ARCHITECTURE.md`는 수정하지 않는다.

# 검증 방법

현재 Target/Type은 허용되고 다른 Target/Type은 거절되며, 사전 검사만으로 Interaction 상태가 바뀌지 않는지 자동화 테스트한다.
