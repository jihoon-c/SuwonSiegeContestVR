# 목적

기존 중앙 `Singijeon_EnemyWave`를 유지하고 좌우에 최적화된 Wave를 한 개씩 추가한다.

# 현재 상태

- 중앙 Wave 1개가 45명을 생성한다.
- 다중 Wave 런타임 예산 자동 분배가 구현되어 있다.
- 기존 레벨 배치 Actor Transform은 변경하면 안 된다.

# 구현 범위

- 중앙 Wave 양옆에 Wave 복제본 추가. 이후 에디터에서 조정된 배치는 그대로 보존
- 중앙 포함 총 3개 Wave, 논리 병력 135명 구성
- 세 Wave가 같은 화차 이벤트와 후퇴 흐름을 사용하도록 유지
- 배치 전후 기존 Actor Transform 불변 검증

# 변경 예정 파일

- `LV_Singijeon.umap`
- `Scripts/ConfigureSingijeonSideEnemyWaves.py`
- `Scripts/VerifySingijeonSideEnemyWaves.py`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 구현 단계

1. 기존 중앙 Wave와 진행 방향 확인
2. 중앙 Wave를 좌우로 복제하고 이후 수동 배치 Transform은 보존
3. 각 복제본의 설정·화차 참조·병력 예산 검증
4. 기존 Actor Transform 불변 확인

# 다른 Feature에 미치는 영향

`GF_Singijeon` 레벨에만 영향을 준다.

# 검증 방법

- 중앙/좌/우 Wave 3개와 총 병력 135명 확인
- Wave당 런타임 Interactive Actor 1명, Pose Leader 2개 예산 확인
- 기존 중앙 Wave와 다른 배치 Actor Transform이 바뀌지 않았는지 비교
- 중앙/좌/우가 서로 다른 위치인지 확인하되 정확한 간격을 강제하지 않음
