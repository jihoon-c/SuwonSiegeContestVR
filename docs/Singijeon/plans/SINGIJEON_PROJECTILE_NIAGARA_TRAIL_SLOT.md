# 목적

발사되는 신기전 화살에 선택 가능한 Niagara 연기 궤적을 연결하고, 화살 기준 위치·회전·크기를 에디터에서 조절할 수 있게 한다.

# 현재 상태

- 완료. 구현 및 검증 결과는 `docs/Singijeon/completed/SINGIJEON_PROJECTILE_NIAGARA_TRAIL_SLOT.md`를 참조한다.

# 구현 범위

- 화살에 비활성 상태의 Niagara Component 추가
- Niagara System 및 상대 Transform 편집 변수 추가
- 발사 시 활성화, 장전·언로드·피격 시 비활성화
- Blueprint 에셋 반영과 자동 검증

# 변경 파일

- `SingijeonProjectileActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- 신기전 설정/검증 스크립트 및 문서

# 구현 단계 (완료)

1. 투사체 Mesh 자식으로 비행 트레일 컴포넌트를 생성한다.
2. Construction 시 에셋과 상대 Transform을 반영한다.
3. 실제 발사 구간에만 이펙트를 활성화한다.
4. 빌드, Blueprint 저장, 자동화 테스트를 수행한다.

# 다른 Feature에 미치는 영향

`GF_Singijeon`의 기본 화살 발사체에만 영향을 준다.

# 검증 방법

- C++ Editor 빌드
- `BP_SingijeonArrow` 컴포넌트 및 기본 비활성 상태 검증
- `GF_Singijeon` 자동화 테스트
