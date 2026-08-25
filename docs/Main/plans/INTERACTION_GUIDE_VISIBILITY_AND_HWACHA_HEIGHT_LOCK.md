# 목적

Scenario 인터랙션 가이드가 로딩 순서와 관계없이 표시되고 실제 HMD 카메라를 바라보게 하며,
신기전 화차 이동 중 장전 완료 시점의 월드 Z 높이를 유지한다.

# 현재 상태

- 가이드 요청 순간 Target Actor를 찾지 못하면 재탐색하지 않는다.
- 빌보드 회전에 180도 보정이 추가되고 PlayerController ViewPoint만 사용한다.
- 화차의 TwoHandCarry는 매 Grip 기준 Z를 사용하고 목표 스냅은 Marker Z를 사용한다.

# 구현 범위

- 활성 Interaction 대상이 늦게 준비돼도 30Hz로 재탐색 후 가이드 표시
- Player Pawn의 활성 CameraComponent를 우선 사용해 가이드 Yaw 갱신
- Widget 정면 축에 맞는 회전으로 수정
- 화차 장전 완료 시점 Z를 Carry와 목표 배치 완료까지 고정

# 변경 예정 파일

- `ScenarioInteractionGuideComponent.h/.cpp`
- `ScenarioFrameworkTests.cpp`
- `TwoHandCarryComponent.h/.cpp`
- `SingijeonHwachaActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- 관련 Core/신기전 문서

# 다른 Feature에 미치는 영향

- Core 가이드 표시 안정화는 공심돈과 다른 Scenario Feature에도 적용된다.
- 높이 고정은 신기전 화차가 명시적으로 설정한 Carry에만 적용된다.

# 검증 방법

- Core Scenario 가이드 지연 대상 및 방향 자동화 테스트
- 신기전 Carry Z 고정 자동화 테스트
- Editor Development 빌드

# 상태

완료 (2026-08-23). Editor Development 빌드와 관련 자동화 테스트 통과.
