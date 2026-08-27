# 목적

적군이 돌진하는 동안 `Troop_march_2`를 재생하고, 발사 후 혼란·후퇴가 시작되면 정지한다.

상태: 완료

# 현재 상태

- Enemy Wave 상태는 `Ready -> Charging -> Panicking -> Retreating` 순서다.
- `Troop_march_2` 사운드 에셋은 존재하지만 Wave와 연결되어 있지 않다.

# 구현 범위

- Wave에 교체 가능한 행군 사운드와 Audio Component 추가
- `Charging` 진입 시 재생 및 원본이 끝나면 상태 유지 중 반복
- `Charging` 이외 모든 상태에서 즉시 정지

# 변경 예정 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 구현 단계

1. Wave Audio Component와 기본 `Troop_march_2` 연결
2. Wave 상태 변경과 재생/정지 동기화
3. 자동화 테스트 및 빌드

# 다른 Feature에 미치는 영향

`GF_Singijeon` Enemy Wave에만 영향을 준다.

# 검증 방법

- `Charging`에서 행군 사운드 재생
- `Panicking`, `Retreating`, `Retreated`, `StopWave`에서 정지
- 기존 Enemy Wave 자동화 테스트 통과
