# 목적

**상태: 완료 (2026-08-21)**

화차 손잡이 원통을 잡아도 이동하지 않는 문제를 수정하고, 도화선 위치를 명확히 안내하며, 도화선 점화 뒤 Scenario의 발사 단계가 열리면 90발이 한 발씩 줄어들며 발사되도록 보장한다.

# 현재 상태

- 자동화 테스트는 운반 컴포넌트를 직접 호출하므로 실제 VR Pawn의 원통 Grab 경로와 Body Mesh Mobility 문제를 검출하지 못한다.
- 도화선은 24cm Trigger지만 시각 가이드가 점화 진행 중에만 표시되어 사전에 위치를 알 수 없다.
- `HandleFuseIgnited()`가 `Hwacha_Fuse`를 완료한 직후 즉시 `LaunchVolley()`를 호출한다. 다음 `Hwacha_Fire` 단계가 아직 열리지 않았다면 발사가 거절되고 재시도되지 않는다.

# 구현 범위

- 실제 Blueprint/레벨의 손잡이, Body Mesh Mobility와 VRGrab 전달 경로 수정
- 도화선 위치 상시 가이드 및 점화 가능/진행 상태 피드백 추가
- Fuse 완료 발사 요청을 보존하고 `Hwacha_Fire` 단계가 활성화될 때 자동 시작
- 발사마다 물리 화살 또는 ISM 한 개를 제거하고 장전 수 Delegate 갱신

# 변경 예정 파일

- `GF_Singijeon`의 Hwacha, Carry, Fuse 관련 C++와 자동화 테스트
- `BP_SingijeonHwacha`, `LV_Singijeon`, 구성/검증 스크립트
- 필요 시 Core VR Grab 탐색과 `docs/Core/`
- 신기전 명세·상태·완료 문서

# 구현 단계

1. 실제 에셋 Mobility/Transform/Grab 범위 진단
2. 원통 Grab과 화차 추종 수정
3. Fuse 위치 가이드 추가
4. Scenario 단계 대기형 자동 발사와 발사별 장전 표시 감소 구현
5. 빌드·자동화·에셋 검증

# 다른 Feature에 미치는 영향

원칙적으로 `GF_Singijeon` 내부에 한정한다. Core Grab 탐색 수정이 필요하면 범용 Primitive 범위 계산만 적용하고 별도 Core 문서에 기록한다. `docs/ARCHITECTURE.md`는 수정하지 않는다.

# 검증 방법

- 실제 VR Grab 핸들 컴포넌트와 Movable Body 검증
- Fuse 완료 시 Fire 단계가 아직 아니어도 요청이 유지되는 자동화 테스트
- 발사 1회마다 총 장전 표시가 90→89→…→0으로 감소하는지 확인
- 도화선 가이드 위치/가시성 에셋 검증
