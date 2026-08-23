# 목적

**상태: 완료 (2026-08-21)**

장전된 화차를 좌우 어느 한 손으로도 이동하고, 표시된 목표 위치에 배치하면 `Hwacha_Aim`을 완료하도록 개선한다.

# 현재 상태

기존 `TwoHandCarry`는 양손이 모두 잡혀야 이동한다. 적군 쪽 다수의 원형은 42개 HISM 대리 적군에 임시 적용된 `SM_MannequinTarget` 형상이다.

# 구현 범위

- 한 손 운반 지원 및 손 전환 시 점프 방지
- 화차와 독립된 월드 목표 마커 표시
- 목표 반경 도착 판정과 시나리오 성공 보고
- 기존 손잡이 Grab 가이드는 유지하고 운반 중 숨김

# 변경 예정 파일

- `TwoHandCarryComponent.h/.cpp`
- `SingijeonHwachaActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- 화차 Blueprint와 `LV_Singijeon`
- 구성·검증 스크립트 및 관련 문서

# 구현 단계

1. 좌우 단일 Grip의 손 이동량으로 화차 이동
2. 장전 시 고정된 목표 마커 표시
3. 목표 반경에 도달하면 마커 숨김 및 `Hwacha_Aim` 완료
4. 빌드, 자동화, 에셋 검증

# 다른 Feature에 미치는 영향

`GF_Singijeon` 내부 운반 컴포넌트와 화차만 변경한다.

# 검증 방법

좌·우 각 단독 Grip 이동 테스트, 손 전환 테스트, 목표 마커 표시·도착 판정, 전체 자동화 테스트.
