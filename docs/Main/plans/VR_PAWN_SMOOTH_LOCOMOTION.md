**상태**: 완료 (2026-08-14)

# 목적

Quest 3에서 왼쪽 조이스틱으로 시점 회전, 오른쪽 조이스틱으로 플레이어 이동을 제공하고 Blueprint에서 이동을 켜고 끌 수 있게 한다.

# 현재 상태

- 기존 `IA_Move`는 텔레포트 입력에 연결되어 있다.
- 기존 기본 매핑은 요청한 좌/우 조이스틱 역할과 다르다.
- 손 AnimBP는 연결되어 있으나 `PoseAlphaGrasp` 값이 전달되지 않는다.

# 구현 범위

- 전용 런타임 Enhanced Input Context 추가
- 왼쪽 조이스틱 X축을 시점 Snap Turn에 연결
- 오른쪽 조이스틱 XY축을 HMD 방향 기준 평면 이동에 연결
- `Enable Move`, 이동 속도, 이동 데드존 변수 추가
- Grab Press/Release 시 손 AnimBP의 `PoseAlphaGrasp` 갱신

# 변경 예정 파일

- `Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`
- 관련 Main 문서

# 구현 단계

1. 런타임 입력 액션과 매핑 컨텍스트를 구성한다.
2. 이동 및 회전 핸들러를 연결한다.
3. Grab 손 애니메이션 값을 연결한다.
4. C++ 및 Blueprint 컴파일을 검증한다.

# 다른 Feature에 미치는 영향

`BP_VRPlayerPawn`을 사용하는 모든 레벨에 동일한 입력이 적용된다. `Enable Move`를 끄면 평면 이동만 막고 시점 회전과 Grab은 유지한다.

# 검증 방법

- Editor 타깃 C++ 빌드
- 전체 Blueprint 컴파일
- 입력 액션/AnimBP 참조 로드 확인
- Quest 3 실기에서 좌 스틱 회전, 우 스틱 이동, Grab 손 포즈 확인
