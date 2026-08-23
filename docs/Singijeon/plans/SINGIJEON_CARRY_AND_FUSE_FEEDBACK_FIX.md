# 목적

장전된 화차의 좌우 손잡이를 한 손으로 잡아도 레벨 충돌 때문에 이동이 막히는 문제와, 점화된 횃불을 도화선에 접촉해도 반응이 보이지 않는 문제를 수정한다.

**상태: 완료 (2026-08-21)**

# 현재 상태

- `UTwoHandCarryComponent`는 손 이동량을 계산하지만 화차 본체를 항상 Sweep 이동해 바닥 등과 접촉한 배치에서는 이동량이 차단될 수 있다.
- `UFuseIgnitionComponent`는 점화 판정과 시간 진행만 담당하며 화차에는 점화 진행을 표시할 VFX가 없다.
- 횃불은 FirePit에서 점화된 경우에만 도화선 점화원으로 인정된다.

# 구현 범위

- 운반 이동의 Sweep 사용 여부를 설정 가능하게 하고 화차는 손 추종 시 Sweep을 끈다.
- 화차 도화선에 점화 진행용 Niagara 컴포넌트를 추가하고 시작·취소·완료 상태와 동기화한다.
- Fuse/횃불 접촉 범위와 충돌 설정을 실제 Blueprint에서 검증한다.

# 변경 예정 파일

- `GF_Singijeon`의 `TwoHandCarryComponent`, `SingijeonHwachaActor`, 자동화 테스트
- `BP_SingijeonHwacha`와 검증/구성 스크립트
- `docs/Singijeon/specs/VR_INTERACTION.md`, `docs/Singijeon/STATUS.md`

# 구현 단계

1. 화차·횃불 Blueprint 컴포넌트 Transform/충돌/상태 진단
2. 화차 한 손 이동의 Sweep 차단 제거
3. Fuse 접촉 중 불꽃 피드백 추가 및 점화 상태 동기화
4. 빌드, 자동화, Blueprint/레벨 에셋 검증

# 다른 Feature에 미치는 영향

변경은 `GF_Singijeon` 내부에 한정한다. Core와 `docs/ARCHITECTURE.md`는 수정하지 않는다.

# 검증 방법

- 충돌 컴포넌트가 있는 월드에서도 한 손 이동량만큼 화차가 이동하는지 자동화 테스트
- 화차 BP에 Fuse VFX와 올바른 Fuse/횃불 Trigger 설정이 있는지 에셋 검증
- 전체 프로젝트 Editor 빌드 및 관련 자동화 테스트
