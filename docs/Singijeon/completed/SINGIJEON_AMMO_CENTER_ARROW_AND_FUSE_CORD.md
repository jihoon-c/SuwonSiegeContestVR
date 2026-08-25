# 화차 장전 배열 편집 화살표 및 도화선 완료

## 작업 결과

- `BP_SingijeonHwacha`의 6 x 15 장전 배열 중앙에 금색 `AmmoGridCenterArrow`를 추가했다.
- 배열 행/열, 간격, Offset과 첫 장전 슬롯 Transform을 기준으로 중심이 자동 갱신된다.
- 화살표는 에디터 뷰포트에서만 보이고 게임 중에는 숨겨진다.
- `FuseCordStart`에서 `Fuse`까지 이어지는 흰색 실 형태 `FuseCord`를 추가했다.
- 도화선은 충돌, Overlap, 그림자를 사용하지 않는다.

## 에디터 조절값

- `Ammo Grid Center Arrow Offset`: 배열 중앙에서 화살표 위치 보정
- `Ammo Grid Center Arrow Rotation`: 화살표 방향 보정
- `Fuse Cord Start`: 도화선 시작점
- `Fuse Cord Radius`: 도화선 굵기

`Fuse` 컴포넌트 위치를 옮기면 도화선 끝점과 길이도 Construction Script 시점에 자동 갱신된다.

## 변경 파일

- `SingijeonHwachaActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- `ConfigureSingijeonAmmoCenterArrowAndFuseCord.py`
- `VerifySingijeonAmmoCenterArrowAndFuseCord.py`
- `/GF_Singijeon/Gameplay/BP_SingijeonHwacha`

## 검증

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공
- Blueprint 구성 및 `LV_Singijeon` 배치 검증 성공
- Map Check: 0 Error, 0 Warning
- `SuwonSiegeContestVR.GF_Singijeon` 자동화 테스트: 6/6 성공
