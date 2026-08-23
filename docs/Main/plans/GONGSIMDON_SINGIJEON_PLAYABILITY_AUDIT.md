# 목적

공심돈과 신기전 체험의 문서·에셋·런타임 구현을 대조하고, 실제 플레이 진행을 막는 미비점을 우선 보완한다.

# 현재 상태

- 두 Feature 모두 Scenario와 Main 왕복 구조는 구현되어 있다.
- 공심돈은 보고 UI/입력과 사격 무기 입력이 연결 대기 상태다.
- 신기전은 핵심 상호작용이 구현됐으나 VFX/SFX, Damage 연동과 Quest 3 실측이 남아 있다.
- 기존 작업 파일이 많은 상태이므로 관련 파일만 최소 수정한다.

# 구현 범위

- 기존 자동화 및 에셋 검증 일괄 실행
- 진행 불가능 또는 시각 피드백 누락 결함 수정
- 공심돈·신기전 레벨과 Scenario 연결 재검증
- 상태/완료 문서 동기화

# 변경 예정 파일

- `Plugins/GameFeatures/GF_Gongsimdon/Source/GF_Gongsimdon/*`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*`
- 관련 구성·검증 스크립트 및 Feature 문서

# 구현 단계

1. 문서와 코드/에셋 대조
2. 빌드, C++ 자동화, Unreal Python 에셋 검증
3. 플레이 차단 결함 우선 수정
4. 회귀 검증과 문서 갱신

# 다른 Feature에 미치는 영향

공통 Scenario/VR 계약은 유지하고 Feature 전용 로직은 각 Game Feature 내부에 둔다.

# 검증 방법

- `SuwonSiegeContestVREditor Win64 Development` 빌드
- Core Scenario, GF_Gongsimdon, GF_Singijeon 자동화
- 공심돈/신기전 Scenario·Level·Blueprint 검증 스크립트

# 완료 결과

- 공심돈에 VR 보고 버튼과 조준 보조 방어 사격 장치를 배치했다.
- 신기전 투사체를 Shared Health의 표준 Damage 계약에 연결했다.
- 구형 원형 화차 마커와 과거 FireEffect 높이를 요구하던 검증 기준을 현행 홀로그램·피벗 보정 설계에 맞췄다.
- Editor 빌드, Core/공심돈/신기전 자동화 13개, 에셋 검증 9종이 통과했다.
