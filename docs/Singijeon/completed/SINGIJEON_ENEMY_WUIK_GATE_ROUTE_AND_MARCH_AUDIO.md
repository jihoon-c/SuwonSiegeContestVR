# 작업

배치된 Enemy Wave 4개가 `BP_Wuik_Gate` 전방으로 이동하도록 목적지를 정렬하고 전진 행군음을 적용했다.

# 구현 내용

- `BP_Wuik_Gate`의 방향과 Bounds를 기준으로 성문 외측 전방 목적지를 계산했다.
- 네 Wave가 한 점에 겹치지 않도록 250cm 간격의 목적지 4개를 배치했다.
- `Singijeon_EnemyWave3`가 성문 반대편으로 향하던 기존 목적지를 수정했다.
- 모든 Wave에 `Troop_march_2`, 볼륨 0.7을 저장했다.
- 행군음은 Wave 상태가 `Charging`일 때만 반복 재생되고, 정지·혼란·처치 완료 상태에서는 정지하는 기존 상태 연동을 유지했다.

# 변경 파일

- `Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap`
- `Scripts/ConfigureSingijeonEnemyWavesToWuikGate.py`
- `Scripts/InspectSingijeonWuikGateAndWaves.py`

# 주요 결정 사항

- 레벨에 배치된 Enemy Wave Actor 4개의 위치, 회전, 스케일은 변경하지 않았다.
- `Destination Actor`는 비운 상태로 유지하고 각 Wave의 `Destination Point`만 성문 전방으로 이동했다.
- 여러 Wave의 동일 음원 중첩을 막기 위해 런타임에는 대표 Wave 하나가 행군음을 재생하는 기존 최적화 구조를 유지했다.

# 테스트 결과

- 레벨 저장 성공
- 레벨 재로드 후 Wave 4개의 목적지 저장 확인
- Wave 4개 모두 `Troop_march_2`, 볼륨 0.7 저장 확인
- `BP_Wuik_Gate` 1개와 Enemy Wave 4개 탐색 확인
- 기존 Wave Actor Transform 보존 검사 통과

# 남은 문제

- 없음
