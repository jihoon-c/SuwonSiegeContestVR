# 작업

신기전 Enemy Wave 식별성, 이동 Animation, 절차별 접근, 편집 가능한 도착점, 발사 후 혼란 이동 보완.

# 구현 내용

- 런타임 적군 Actor를 `SingijeonEnemy_XX`로 생성하고 에디터 Label도 동일한 순번으로 표시한다.
- 레벨 인스턴스의 Run Animation이 비어 있어도 Manny Rifle Jog Animation을 런타임 Fallback으로 사용한다.
- 전경 병사 Mesh 설정을 Animation 유무와 분리해 마네킹 Mesh가 항상 적용되도록 했다.
- 화차는 이벤트 소스로만 연결하고, 최종 이동 지점은 `DestinationActor` 또는 `DefaultTargetPoint`를 사용한다.
- 시작 시 화차의 현재 장전·조준·점화·발사 상태를 읽어 접근 단계를 동기화한다.
- 장전 35%, 조준 60%, 점화 82%, 발사 95% 접근 제한을 유지한다.
- 발사 이벤트에서 즉시 제거하지 않고 `Panicking` 상태로 전환해 시드 기반 좌우 산개·후퇴·회전을 수행한 뒤 명중 판정한다.
- 기존 레벨과 배치 Actor Transform은 수정하거나 재저장하지 않았다.

# 변경 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- `docs/Singijeon/plans/SINGIJEON_ENEMY_WAVE_DESTINATION_AND_PANIC.md`

# 주요 결정 사항

- 45개 개별 AI/PathFollowing 대신 기존 Wave 관리자 Tick 하나에서 Panic Offset까지 계산해 VR 성능 구조를 유지한다.
- 기존 직렬화 호환을 위해 `TargetActor`는 화차 참조로 유지하고, 이동 목적지는 별도 `DestinationActor`로 분리한다.
- 목적지 Actor를 지정하지 않으면 배치된 Enemy Wave의 `Destination Point` 컴포넌트를 뷰포트에서 이동해 설정한다.

# 테스트 결과

- Unreal Header Tool: 성공.
- `SuwonSiegeContestVR Win64 Development -Module=GF_Singijeon`: 컴파일 성공.
- `SuwonSiegeContestVREditor Win64 Development`: 컴파일 및 DLL 링크 성공.
- `EnemyWave.EditorPreview`, `EnemyWave.ScaleAndVolley`: 자동화 테스트 2개 성공.
- 반복 `PrepareWave` 시 생성 이름 충돌이 발생하지 않도록 실제 Level Outer 기준의 고유 이름 생성을 검증했다.
- `git diff --check`: 공백 오류 없음.

# 남은 문제

- 없음.
