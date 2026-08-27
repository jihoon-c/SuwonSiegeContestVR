# 작업

신기전 발사 후 적군 혼란/후퇴와 Enemy Wave 복제 대응 VR 최적화

# 구현 내용

- `Panicking -> Retreating -> Retreated` 상태 전이를 추가했다.
- 기본 발사 사상률을 35%로 조정하고 생존자는 6초 동안 Route 반대 방향으로 3500cm 후퇴한다.
- 후퇴 완료 시 병사 메시를 숨기고 Skeletal Animation Tick을 중지한다.
- Enemy Wave를 복제하면 Actor당 45명이 추가되고 각 Wave가 같은 플레이 가능 화차 이벤트를 독립적으로 수신한다.
- 다중 Wave에서는 전경 Enemy Actor와 Pose Leader 예산을 자동 분배한다. 두 Wave 기준 각 2 Actor/3 Leader다.
- 복제 위치를 Formation Seed에 혼합해 복제본의 정배치 반복을 줄였다.
- Proxy Transform 갱신을 10Hz에서 8Hz로 낮추고 단일 Wave Pose Leader를 8개에서 6개로 줄였다.
- Quest/OpenXR 가시성 문제가 있었던 독립 Enemy Actor 프록시 구조는 유지했다.
- 설정 스크립트가 기존 Wave 복제본을 삭제하거나 배치 Transform을 변경하지 않도록 수정했다.

# 변경 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- `Scripts/ConfigureSingijeonEnemyWave.py`
- `Scripts/VerifySingijeonEnemyWave.py`
- `Scripts/VerifySingijeonSamuraiEnemyWave.py`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 주요 결정 사항

- Wave 복제는 병력 수를 실제로 늘리되 고비용 Actor/애니메이션 평가는 자동 분배한다.
- 레벨 배치 위치는 수정하지 않았다.
- 실험적 GPU Instanced Crowd는 Quest 표시 안정성 때문에 활성화하지 않았다.

# 테스트 결과

- UnrealBuildTool 전체 에디터 타깃: 성공
- `EnemyWave.EditorPreview`: 성공
- `EnemyWave.MultiWaveBudget`: 성공
- `EnemyWave.ScaleAndVolley`: 성공
- 배치 Wave 속성/화차 참조 검증: 성공
- Samurai LOD/리타겟 Run/Pose 공유 설정 검증: 성공

# 남은 문제

- 현재 비활성인 GPU Crowd Provider에는 과거 애니메이션 트랙이 남아 있다. `Use GPU Instanced Crowd=false`인 현재 Quest-safe 경로에는 영향이 없으며, GPU 경로를 다시 활성화할 때 Provider를 재생성해야 한다.
- IK Retarget 저작용 Asset 3개는 현재 Asset Registry에 등록되지 않았지만, 생성 완료된 Samurai Run 애니메이션의 런타임 재생에는 영향이 없다.
