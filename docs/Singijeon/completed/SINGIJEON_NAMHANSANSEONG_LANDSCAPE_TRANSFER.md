# 신기전 남한산성 Landscape 이식 완료

## 결과

- `Demo_Namhansanseong` 중앙 지형을 `LV_Singijeon`의 기존 Landscape에 이식했다.
- 원본 중앙 Proxy의 255x255 16비트 높이 데이터를 505x505로 보간했다.
- 원본의 `Forest_LayerInfo`, `Grass_LayerInfo`, `Ground_LayerInfo` Weightmap도 함께 보간해 기본 갈색 레이어 노출을 제거했다.
- World Partition Proxy 121개와 데모 장식 Actor는 가져오지 않아 신기전 레벨은 단일 Landscape 구조를 유지한다.
- Landscape 머티리얼은 `/Game/Namhansanseong/Materials/Landscape/MI_Landscape`를 사용한다.
- Landscape Scale은 `(100, 100, 60)`, 중앙 지면이 Z=0이 되도록 Actor Z는 `-271.40625`로 정렬했다.
- VR 비용 제한을 위해 `MaxLODLevel=2`로 설정했다.

## 재실행 도구

- 콘솔 명령: `Suwon.TransferNamhansanseongLandscape`
- 실행 스크립트: `Scripts/RunNamhansanseongLandscapeTransfer.py`
- 검증 스크립트: `Scripts/VerifyNamhansanseongLandscapeTransfer.py`

## 검증

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공
- 저장 후 레벨 재로드 성공
- Landscape Bounds Z Extent: `1706.015625`로 비평면 확인
- `MI_Landscape`, `MaxLODLevel=2`, `NamhansanseongTerrain` 태그 확인
- Forest/Grass/Ground Weightmap 3개 레이어 저장 확인
- Scenario Manager, Hwacha, Torch, FirePit, EnemyWave, PlayerStart 보존 확인

## 주의

Landscape 머티리얼은 `/Game/Namhansanseong` 에셋을 참조하므로 해당 폴더를 삭제하거나 이동하면 머티리얼 참조를 함께 수정해야 한다.
