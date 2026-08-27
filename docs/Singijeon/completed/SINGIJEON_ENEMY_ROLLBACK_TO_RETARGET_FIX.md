# 작업

적군 시스템을 Samurai 리타게터 프리뷰 수정 완료 직후 시점으로 복원.

# 구현 내용

- `LV_Singijeon`의 중앙 `Singijeon_EnemyWave`와 Transform을 유지했다.
- 이후 추가됐던 좌/우 측면 Enemy Wave를 제거했다.
- `Panicking -> Retreating -> Retreated` 후퇴 흐름을 제거하고 당시 `Panicking -> Defeated` 흐름으로 복원했다.
- 다중 Wave 자동 Actor/Pose 예산 분배와 위치 기반 Formation Seed 혼합을 제거했다.
- 단일 Wave 값을 45명, Proxy Scale 0.9, 10Hz 갱신, Pose Leader 8개, Volley 사상률 100%로 복원했다.
- 정상화된 Samurai 메시, IK/Retarget 결과 달리기 애니메이션과 Quest-safe 독립 Actor 프록시는 유지했다.
- 최근 요구사항인 돌진 중 행군 사운드는 유지했다.

# 변경 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- `Scripts/ConfigureSingijeonEnemyWave.py`
- `Scripts/VerifySingijeonEnemyWave.py`
- `Scripts/VerifySingijeonSamuraiEnemyWave.py`
- `Scripts/RollbackSingijeonEnemyWaveToRetargetFix.py`
- `LV_Singijeon.umap`

# 주요 결정 사항

- 다른 Gameplay 기능과 기존 Actor Transform은 변경하지 않았다.
- `SINGIJEON_ENEMY_RETREAT_AND_MULTI_WAVE` 이후의 적군 변경만 되돌렸다.
- `docs/ARCHITECTURE.md`는 수정하지 않았다.

# 테스트 결과

- UnrealBuildTool `SuwonSiegeContestVREditor Win64 Development`: 성공.
- `EnemyWave.EditorPreview`: 성공.
- `EnemyWave.ScaleAndVolley`: 성공.
- 레벨 내 Enemy Wave 1개, 45명, Scale 0.9, 10Hz, Pose Leader 8, Volley 100% 검증: 성공.
- Samurai 최적화 메시, 리타겟 Run Animation, LOD1, Quest-safe 렌더러 참조 검증: 성공.
- 중앙 Wave 및 모든 존속 Actor Transform 불변 검사: 성공.

# 남은 문제

- 사용하지 않는 GPU Provider에는 삭제된 과거 시퀀스 참조 경고가 남지만 현재 `Use GPU Instanced Crowd=false` 경로에는 영향이 없다.
