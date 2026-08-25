# 목적

신기전 적군 Wave의 런타임 표시·애니메이션 문제를 해결하고, 절차별 접근과 발사 후 혼란 이동을 하나의 경량 Wave 관리자에서 처리한다.

**상태: 완료 (2026-08-25)**

# 현재 상태

- 생성 적군 Actor 이름이 자동 생성되어 Outliner에서 식별하기 어렵다.
- 레벨 인스턴스에 달리기 Animation이 `None`으로 저장되면 마네킹이 이동만 하고 자세는 정지한다.
- 화차 자동 연결 시 경로 도착점까지 화차로 덮어써서 에디터용 기본 도착점을 쓰기 어렵다.
- 장전·조준·점화·발사 단계별 접근 제한은 있으나, 발사 이벤트가 즉시 전투 판정을 수행해 혼란 연출이 보이지 않는다.

# 구현 범위

- 생성 적군에 `SingijeonEnemy_XX` 이름과 에디터 Label 부여
- Manny 호환 달리기 Animation의 런타임 Fallback 제공
- 화차 이벤트 연결과 이동 도착점을 분리하고 `DefaultTargetPoint`를 에디터에서 직접 이동 가능하게 유지
- 기존 절차별 접근 상한을 유지하고 현재 화차 상태를 시작 시 동기화
- 신기전 발사 후 일정 시간 대형을 흩뜨리며 이동한 뒤 명중 판정
- 기존 레벨 배치 Actor의 Transform과 `docs/ARCHITECTURE.md`는 수정하지 않음

# 변경 예정 파일

- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Public/Enemy/SingijeonEnemyWaveActor.h`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Enemy/SingijeonEnemyWaveActor.cpp`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonEnemyWaveTests.cpp`
- 신기전 완료 기록 문서

# 구현 단계

1. 도착점과 화차 이벤트 소스를 분리한다.
2. 런타임 Animation Fallback과 식별 가능한 Actor 이름을 적용한다.
3. 발사 시 Wave 단위 Panic 상태와 시드 기반 산개 Offset을 적용한다.
4. 절차 단계, 도착점, Panic 지연 판정을 자동화 테스트로 검증한다.
5. 플러그인 컴파일 및 관련 테스트를 수행한다.

# 다른 Feature에 미치는 영향

`GF_Singijeon`과 기존 Shared `EnemySoldierActor` 사용 범위에 한정한다. Shared 클래스 자체는 수정하지 않는다.

# 검증 방법

- 45개 적군이 식별 가능한 이름으로 생성되고 Run Animation을 갖는지 확인
- 장전 35%, 조준 60%, 점화 82%, 발사 95% 접근 상한 확인
- `DestinationActor` 또는 `DefaultTargetPoint`가 최종 경로점으로 사용되는지 확인
- 발사 직후 적이 즉시 제거되지 않고 Panic 상태로 이동한 뒤 판정되는지 확인
- Win64 Development Editor 컴파일과 EnemyWave 자동화 테스트 통과 확인
