# 작업

Android 스탠드얼론 VR용 Shared Enemy AI LOD 및 Actor Pool 기반 구현

# 구현 내용

- `AEnemyCombatCharacter`: Enemy 진영, `AEnemyAIController`, AI LOD/단순 이동/행동 상태 Component 기본 구성
- `UEnemyAILODComponent`: 거리 히스테리시스 기반 원거리/근거리 전환. 원거리 메시 숨김·저빈도 Actor Tick·BT 중지, 근거리 메시 표시·BT 활성화
- `UEnemySimpleMovementComponent`: NavMesh/BT 없이 목표 Actor 또는 목표 위치로 직접 이동하는 저비용 원거리 이동
- `UEnemyBehaviorStateComponent`: Feature가 `Advance`, `Construct`, `Assault`, `Retreat`, `Archer` 등 상태를 전달할 수 있는 이벤트 계약
- `AEnemyAIController`: 근거리에서만 선택한 Behavior Tree를 실행. StateTree Feature는 `OnHighDetailAIChanged` 이벤트를 연결 가능
- `AActorPool`, `IPoolableActorInterface`: 사전 생성과 재사용으로 Spawn/Destroy 스파이크를 방지하는 계약

# 시나리오 연결 방식

- 웅성/총통: `Advance`로 성문 목표를 향해 이동하고, 범위 진입 이벤트에서 `Construct`, 충차 완성 시 `Assault`로 전환한다. 궁병은 근거리 BT/StateTree에서 아군 성벽 표적 사격을 담당한다.
- 공심돈: 야간 접근 중에는 `Advance`, 플레이어에게 발각되면 `Retreat`로 전환해 반대편 탈출 목표를 `SetMoveTargetLocation`으로 지정한다.
- 녹뢰: 적을 생성하지 않으므로 이 계층을 사용하지 않는다.
- 신기전: `Advance` 상태와 고정 목표점만 지정해 일방향 달리기를 수행한다.

# 성능 결정

- 풀은 기본 확장 금지다. Feature마다 최다 동시 적/투사체 수를 InitialPoolSize로 사전 할당한다.
- 원거리 적은 0.5초 기본 주기와 직접 이동으로 처리하며, 메시와 고비용 AI를 비활성화한다.
- 근거리 임계값과 풀 크기는 Android 실기기 프로파일링으로 최종 조정한다.

# 테스트 결과

- `UnrealBuildTool SuwonSiegeContestVREditor Win64 Development -NoHotReload` 성공
- UHT 및 C++ 23개 작업 성공

# 남은 문제

- 각 Feature의 Behavior Tree/StateTree Asset, 스폰 매니저, 목표점 배치, 메시/애니메이션, 실기기 성능 수치 측정은 별도 구현이 필요하다.
