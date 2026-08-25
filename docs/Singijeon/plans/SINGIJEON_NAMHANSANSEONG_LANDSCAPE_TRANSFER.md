# 목적

`/Game/Namhansanseong/Maps/Demo_Namhansanseong`의 Landscape 지형을 `LV_Singijeon`에 이식해 신기전 체험 배경으로 사용한다.

상태: 완료

# 현재 상태

- 원본은 World Partition 외부 Actor를 사용하는 남한산성 데모 레벨이다.
- 대상은 `/GF_Singijeon/Maps/LV_Singijeon`이며 게임플레이 Actor가 이미 배치돼 있다.
- 원본은 1개 Landscape Root와 121개 Streaming Proxy로 구성된 약 280km급 지형임을 확인했다.
- 대상에는 64 Component, 505x505 샘플의 단일 평면 Landscape가 있었다.

# 구현 범위

- 원본 Landscape 및 의존 머티리얼/레이어 확인
- 중앙 Streaming Proxy의 255x255 16비트 높이 및 Forest/Grass/Ground Weightmap을 기존 505x505 Landscape로 보간 이식
- 기존 신기전 게임플레이 Actor와 시나리오 설정 보존
- VR 성능에 불필요한 데모 Actor는 이식하지 않음

# 변경 파일

- `Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap`
- `Source/SuwonSiegeContestVREditor/Private/SuwonSiegeContestVREditor.cpp`
- `Source/SuwonSiegeContestVREditor/SuwonSiegeContestVREditor.Build.cs`
- `Scripts/RunNamhansanseongLandscapeTransfer.py`
- `Scripts/VerifyNamhansanseongLandscapeTransfer.py`
- 완료 문서

# 구현 단계

1. 원본/대상 레벨의 Landscape 및 월드 형식을 계측했다.
2. World Partition Actor 복제 대신 16비트 높이 샘플 직접 전송 방식을 선택했다.
3. 원본을 수정하지 않고 기존 대상 Landscape의 첫 Edit Layer에 높이와 Forest/Grass/Ground 페인트 레이어를 기록했다.
4. 재로드 후 비평면 Bounds, 머티리얼, LOD 및 게임플레이 Actor 보존을 검증했다.

# 다른 Feature에 미치는 영향

`GF_Singijeon` 레벨에만 적용하며 Core 및 다른 Game Feature는 수정하지 않는다. `/Game/Namhansanseong` 원본 에셋은 참조 전용으로 유지한다.

# 검증 방법

- Editor 빌드 성공
- 대상 레벨 저장 후 재로드 성공
- Landscape Bounds Z Extent `1706.015625`, 머티리얼 `MI_Landscape`, Max LOD `2` 및 3개 Weightmap 레이어 확인
- Hwacha, FirePit, Torch, EnemyWave, Scenario Manager, PlayerStart 보존 확인
