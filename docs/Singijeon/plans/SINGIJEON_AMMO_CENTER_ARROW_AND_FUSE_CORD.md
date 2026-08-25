# 목적

화차의 6x15 장전 배열 중심과 방향을 에디터에서 쉽게 확인하고, 도화선 접촉 위치를 흰색 실 형태로 시각화한다.

# 현재 상태

- 완료. 구현 및 검증 결과는 `docs/Singijeon/completed/SINGIJEON_AMMO_CENTER_ARROW_AND_FUSE_CORD.md`를 참조한다.

# 구현 범위

- 게임 중 숨겨지는 에디터 전용 배열 중심 화살표
- 충돌과 그림자가 없는 흰색 실 형태 도화선 메시
- Blueprint/레벨 에셋 반영과 자동 검증

# 변경 파일

- `SingijeonHwachaActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- 신기전 설정/검증 스크립트 및 문서

# 구현 단계 (완료)

1. 배열 중심을 Rows/Columns/Spacing/Offset으로 계산한다.
2. `UArrowComponent`를 중심에 배치하고 실제 화살 방향을 표시한다.
3. 얇은 Cylinder 메시를 Fuse까지 연결하고 흰색 머티리얼을 적용한다.
4. 빌드, 에셋 저장, 테스트를 수행한다.

# 다른 Feature에 미치는 영향

`GF_Singijeon`의 화차 Actor에만 영향을 준다.

# 검증 방법

- C++ 빌드 및 `GF_Singijeon` 자동화 테스트
- `BP_SingijeonHwacha`, `LV_Singijeon` 컴포넌트/Map Check
