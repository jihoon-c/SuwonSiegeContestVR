# 목적

관련 나레이션과 Scenario 순서가 오기 전에 신기전을 화차에 장전하거나 횃불·도화선을 점화해 진행이 멈추는 문제를 방지한다.

**상태: 완료 (2026-08-21)**

# 현재 상태

- 화살 슬롯은 겹치면 즉시 장전한 뒤 완료 보고한다.
- FirePit은 완료 보고를 먼저 시도하지만 Core 로컬 완료 이벤트가 Manager 승인 전에 발생한다.
- Fuse는 화차 장전 상태만 확인하고 현재 Scenario 단계는 확인하지 않는다.

# 구현 범위

- `Hwacha_Load`가 현재 단계일 때만 장전
- `Torch_Ignite`가 현재 단계일 때만 횃불 점화
- `Hwacha_Fuse`가 현재 단계일 때만 Fuse 점화 시작
- `Hwacha_Fire` 단계가 아니면 발사 시작 차단

# 변경 예정 파일

- GF_Singijeon 장전 슬롯, FirePit, Fuse, Hwacha 및 자동화 테스트
- 신기전 명세/상태 문서

# 구현 단계

1. 각 물리 상태 변경 직전에 Scenario 사전 허용 검사
2. 거절 시 Attach, 점화 상태, VFX, 발사 상태를 변경하지 않음
3. 정상 순서와 조기 행동 자동화 검증

# 다른 Feature에 미치는 영향

신기전 Feature 내부 동작만 변경한다. 공통 사전 검사 API는 Core가 제공하며 `docs/ARCHITECTURE.md`는 수정하지 않는다.

# 검증 방법

다른 Interaction이 Running일 때 장전/점화/Fuse가 거절되고, 해당 Interaction으로 이동한 뒤 동일 행동이 성공하는지 검사한다.
