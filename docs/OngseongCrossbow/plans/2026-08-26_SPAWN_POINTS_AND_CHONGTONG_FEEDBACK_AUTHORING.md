# 목적

**상태: Completed (2026-08-26)** — 구현 결과는 `../completed/2026-08-26_SPAWN_POINTS_AND_CHONGTONG_FEEDBACK_AUTHORING.md`에 기록했다.

옹성 전투의 적 초기 스폰, 병사 리스폰, 충차 스폰 위치를 레벨에서 명시적으로 배치할 수 있게 하고 총통의 발사/폭발 피드백을 블루프린트에서 교체 가능하게 한다.

# 현재 상태

- `AOngseongEnemyWaveManager`의 액터 Transform이 적 초기 스폰과 리스폰에 함께 사용된다.
- `AOngseongDefenseScenarioManager`는 일반 `AActor`인 `RamSpawnPoint`를 사용하며, 비어 있으면 웨이브 매니저 Transform으로 폴백한다.
- `AChongtongCannonActor`에 `MuzzleEffect`, `FireSound`가 있고 `AChongtongProjectileActor`에 `ExplosionEffect`, `ExplosionSound`가 있으나 블루프린트 그래프에서는 읽기 전용이다.

# 구현 범위

- 역할 선택형 `AOngseongSpawnPointActor` C++ 기반 클래스 및 `BP_OngseongSpawnPoint` 블루프린트 생성
- 최초 적 스폰과 병사 리스폰 위치 분리
- 충차 스폰 포인트 자동 탐색
- 포구 화염, 포격음, 폭발 이펙트, 폭발음을 `BlueprintReadWrite`로 노출

# 변경 예정 파일

- `Public/Ongseong/OngseongSpawnPointActor.h`
- `Private/Ongseong/OngseongSpawnPointActor.cpp`
- `OngseongEnemyWaveManager.h/.cpp`
- `OngseongDefenseScenarioManager.cpp`
- `ChongtongCannonActor.h`
- `ChongtongProjectileActor.h`
- `/GF_OngseongCrossbow/Gameplay/Waves/BP_OngseongSpawnPoint`

# 구현 단계

1. 역할과 에디터 프리뷰를 가진 스폰 포인트 액터 추가
2. 웨이브/시나리오 매니저의 명시적 참조 및 역할 기반 자동 탐색 연결
3. 최초 스폰과 리스폰 Transform 사용 경로 분리
4. 총통 피드백 속성 쓰기 노출
5. C++ 빌드, Blueprint 컴파일, 에셋 속성 검증

# 다른 Feature에 미치는 영향

변경은 `GF_OngseongCrossbow` 내부에 한정되며 Core/Shared Gameplay 의존 방향은 바뀌지 않는다. 기존 레벨 참조가 없는 경우에는 종전의 매니저 Transform 폴백을 유지한다.

# 검증 방법

- Development Editor 빌드
- `BP_OngseongSpawnPoint` 컴파일
- 기존 옹성 Blueprint 3종의 피드백 속성 조회
- 옹성 자동화 테스트 실행 가능 여부 및 에디터 로그 오류 확인
