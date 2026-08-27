# 작업

`LV_Singijeon`에서 적군 캐릭터가 다시 보이지 않는 현상이 이전 OpenXR Scene Proxy 누락과 같은 문제인지 점검했다.

# 구현 내용

- 중앙/좌/우 Wave 전체를 대상으로 런타임 가시성 계측 스크립트를 추가했다.
- Oculus OpenXR 런타임을 실제 실행해 플레이어 시작 시점의 프레임을 캡처했다.
- 좌우 Wave 검증은 사용자가 에디터에서 조정한 Transform을 보존하도록 정확한 간격을 강제하지 않게 변경했다.

# 변경 파일

- `Scripts/InspectSingijeonEnemyVisibilityAllWaves.py`
- `Scripts/VerifySingijeonSideEnemyWaves.py`
- `docs/Singijeon/plans/SINGIJEON_SIDE_ENEMY_WAVES.md`

# 주요 결정 사항

레벨에 배치된 기존 Actor Transform은 수정하지 않았다. 이번 현상은 렌더 Actor 생성 실패가 아니라 현재 플레이어 시점과 적군 사이의 성벽에 의한 완전 가림이다.

# 테스트 결과

- Wave 3개, 논리 병력 135명 생성 성공
- 135개 모두 `EnemySoldierActor`, `Hidden=false`, Mesh `HiddenInGame=false`, Main Pass 활성
- Wave 3개 모두 `Ready`, `Show Enemies While Ready=true`, GPU Instanced Crowd 비활성
- Oculus OpenXR 초기 프레임에서 플레이어와 적군 사이에 성벽이 위치함을 확인
- PlayerStart Z `695.97`, Enemy Wave Z `82.18`: 약 `613.79cm` 높이 차이

# 남은 문제

적군을 시작 위치에서 보여야 한다면 레벨 배치 의도가 필요하다. PlayerStart, 성벽, 적군 중 어느 쪽도 임의로 이동하지 않았다.
