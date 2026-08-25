# 작업

신기전 화차의 장전 배열을 기존 6행 × 15열에서 6행 × 11열로 줄였다.

# 구현 내용

- 자동 장전 열 수를 11로 변경하고 행 수는 6으로 유지했다.
- 완전 장전 수를 66발로 변경했다.
- Blueprint 기본값과 `LV_Singijeon` 배치 액터 값을 실제 에셋에 저장했다.
- 배치 액터의 `AutoLoadedArrowInstances`를 `RackRoot` 로컬 원점에 정규화했다.
- 발사 간격과 잔탄 테스트를 66발 기준으로 갱신했다.

# 변경 파일

- `BP_SingijeonHwacha.uasset`
- `LV_Singijeon.umap`
- `SingijeonHwachaActor.h`
- `SingijeonHwachaTests.cpp`
- 신기전 화살 배열 구성/검증 스크립트

# 주요 결정 사항

플레이어가 직접 꽂는 첫 화살을 포함해 총 66발이며, 자동 생성 ISM은 나머지 65개다.

# 테스트 결과

- `SuwonSiegeContestVR.GF_Singijeon` 자동화 테스트 7개 성공
- 에셋 검증: Blueprint와 레벨 액터 모두 6행, 11열, 66발 성공
- 에셋 검증: 화살 ISM 머터리얼과 `RackRoot` 로컬 고정 성공

# 남은 문제

없음.
