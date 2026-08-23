# 목적

**상태: 완료 (2026-08-21)**

화차 장전 화살 인스턴스에 머터리얼이 보이지 않는 문제를 수정한다.

# 현재 상태

`M_Arrow01b` 참조는 존재하지만 부모가 `/InterchangeAssets/gltf/Substrate/M_GLTF`여서 런타임·ISM 표시와 Cook 의존성이 불안정하다.

# 구현 범위

- 프로젝트 소유 화살 Material 생성
- Static Mesh, 투사체, 자동 장전 ISM에 동일 Material 적용
- Blueprint와 배치 액터 참조 검증

# 변경 예정 파일

- 신기전 화살 Material/Static Mesh/Blueprint/Level 에셋
- 자동 장전 구성·검증 스크립트

# 구현 단계

1. 기존 텍스처로 독립 Material 생성
2. 모든 화살 렌더링 경로에 적용
3. 에디터 검증 및 자동화 테스트

# 다른 Feature에 미치는 영향

`GF_Singijeon` 내부 에셋만 변경한다.

# 검증 방법

Material 부모 의존성, Static Mesh 슬롯, 투사체 슬롯, CDO/배치 ISM 슬롯을 자동 검사한다.
