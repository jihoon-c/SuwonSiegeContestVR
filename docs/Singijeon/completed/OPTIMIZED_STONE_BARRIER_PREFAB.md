# 작업

`SM_Stone_Barrier` 기반 성능 최적화 벽 프리팹 생성

# 구현 내용

- 원본 벽 메시의 형상과 충돌은 유지한다.
- 별도 복제한 Base Color 텍스처를 최대 1024px로 제한했다.
- Normal/Roughness 샘플 없이 Base Color 한 장만 쓰는 `M_Stone_Barrier_Low` Fully Rough 머터리얼을 만들었다.
- 두 Material Slot에 같은 경량 머터리얼을 적용한 `BP_Stone_Barrier_Optimized`를 생성했다.
- 3500cm 거리 컬링을 지정했다.

# 변경 파일

- `GF_Singijeon/Content/Asset/NamhansanseongWall/Optimized/*`
- `Scripts/CreateOptimizedStoneBarrierPrefab.py`
- `Scripts/VerifyOptimizedStoneBarrierPrefab.py`

# 주요 결정 사항

원본 `SM_Stone_Barrier`와 고품질 머터리얼은 수정하지 않는다. 가까운 핵심 구간에는 원본을, 반복 배치·원거리 벽에는 최적화 프리팹을 사용한다.

# 테스트 결과

- 1K 텍스처, Fully Rough 머터리얼, 원본 메시 할당, 두 슬롯 머터리얼 오버라이드, 3500cm 컬링 검증 성공

# 남은 문제

Quest 3에서 원거리 벽의 3500cm 컬링 경계를 실제 씬 규모에 맞게 조절할 수 있다.
