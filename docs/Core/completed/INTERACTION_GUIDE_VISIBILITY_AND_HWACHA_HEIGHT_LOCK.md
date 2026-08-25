# 작업

Scenario 인터랙션 가이드 누락·방향 문제와 신기전 화차 이동 높이 변경을 수정했다.

# 구현 내용

- 활성 Interaction 요청 시 Target Actor가 아직 없으면 가이드를 포기하지 않고 30Hz로 재탐색한다.
- Target Actor가 스트리밍 또는 Feature 활성화로 늦게 생성돼도 즉시 가이드를 표시한다.
- PlayerController보다 Player Pawn의 활성 HMD CameraComponent 위치를 우선 사용한다.
- WidgetComponent의 실제 정면인 로컬 +X가 플레이어를 향하도록 잘못된 180도 회전을 제거했다.
- 가이드 표시 시 HiddenInGame과 Visibility를 함께 복원하고 Translucent Sort Priority를 높였다.
- 화차는 장전 완료 시점 Z를 Carry 제약 높이로 저장한다.
- 한손/양손 전환과 손의 상하 이동 중에도 저장된 Z를 유지한다.
- 목표 홀로그램 도착 시에도 XY/Yaw만 적용하고 저장된 Z는 유지한다.

# 변경 파일

- `ScenarioInteractionGuideComponent.h/.cpp`
- `ScenarioFrameworkTests.cpp`
- `TwoHandCarryComponent.h/.cpp`
- `SingijeonHwachaActor.cpp`
- `SingijeonHwachaTests.cpp`
- Scenario 및 신기전 상호작용 명세

# 주요 결정 사항

- 가이드 재탐색은 활성 Interaction 동안만 수행하여 유휴 상태 비용을 만들지 않는다.
- 화차 높이 잠금은 `TwoHandCarryComponent.SetConstrainedWorldZ`를 화차가 명시적으로 설정하므로 다른 Carry Actor에는 영향을 주지 않는다.
- `docs/ARCHITECTURE.md`는 수정하지 않았다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development`: 성공
- 프로젝트 전체 자동화 테스트 26개: 전부 성공

# 남은 문제

- Quest 3에서 실제 HMD 기준 가이드 정면과 화차 바퀴의 바닥 접촉을 육안 확인한다.
