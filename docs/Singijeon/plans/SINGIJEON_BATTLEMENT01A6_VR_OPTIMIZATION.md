# 목적

`LV_Singijeon`에 배치된 `SM_Battlement01a6` 성벽을 Quest/OpenXR VR에 적합하게 최적화한다.

# 현재 상태

- 원본 메시가 `GF_OngseongCrossbow` Feature 폴더에 있다.
- 실제 배치 수, LOD, 머터리얼, 충돌 및 그림자 설정은 Unreal에서 확인이 필요하다.
- 기존 배치 Actor Transform은 변경하면 안 된다.

# 구현 범위

- 원본 에셋은 수정하지 않고 `GF_Singijeon` 내부에 VR 전용 복제본을 만든다.
- LOD0 외형은 유지하고 거리별 경량 LOD를 생성한다.
- 필요 시 경량 머터리얼과 단순 충돌/그림자 설정을 적용한다.
- `LV_Singijeon`에서 해당 메시를 사용하는 컴포넌트만 VR 복제본으로 교체한다.
- Actor 위치, 회전, 스케일은 전후 검증하여 그대로 유지한다.

# 변경 예정 파일

- `GF_Singijeon/Content/Art/VR_Optimized/SM_Battlement01a6_VR.uasset`
- 필요 시 경량 머터리얼/텍스처 에셋
- `LV_Singijeon.umap`
- 생성 및 검증 Python 스크립트

# 구현 단계

1. 배치 수와 원본 메시 비용을 검사한다.
2. VR 전용 메시와 LOD를 생성한다.
3. 배치 컴포넌트 참조만 교체한다.
4. Transform 불변, LOD 및 참조를 검증한다.

# 다른 Feature에 미치는 영향

원본 `GF_OngseongCrossbow` 에셋은 변경하지 않아 다른 레벨에는 영향이 없다.

# 검증 방법

- Unreal Python 메시/레벨 검사
- 원본 참조 0개 및 VR 메시 참조 확인
- 교체 대상 Actor Transform 전후 비교
- 에셋 로드 및 LOD 개수 확인
