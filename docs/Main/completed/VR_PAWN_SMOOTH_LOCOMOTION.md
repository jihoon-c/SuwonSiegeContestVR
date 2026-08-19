# 작업

Quest 3용 `BP_VRPlayerPawn`에 좌 스틱 시점 회전, 우 스틱 평면 이동, 이동 On/Off 설정과 Grab 손 포즈를 연결했다.

# 구현 내용

- 런타임 Enhanced Input Context를 우선순위 100으로 추가했다.
- 왼쪽 컨트롤러 조이스틱 X축은 기존 Snap Turn에 연결했다.
- 오른쪽 컨트롤러 조이스틱 XY축은 HMD Yaw 기준 전후좌우 이동에 연결했다.
- `Enable Move`가 꺼지면 평면 이동만 차단하고 Turn, Grab은 유지한다.
- `Smooth Move Speed` 기본값은 초당 180cm, Dead Zone 기본값은 0.15다.
- Grab Press/Release 시 양손 AnimBP의 `PoseAlphaGrasp`를 1/0으로 갱신한다.
- UE 5.8 Blueprint 실수 변수가 `double`로 생성되는 경우까지 처리하도록 `FNumericProperty`를 사용한다.

# 변경 파일

- `Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`
- `Source/SuwonSiegeContestVR/Private/Tests/ScenarioFrameworkTests.cpp`
- `docs/Main/plans/VR_PAWN_SMOOTH_LOCOMOTION.md`

# 주요 결정 사항

- 기존 텔레포트용 `IA_Move`와 충돌하지 않도록 현재 Pawn 전용 런타임 액션을 사용한다.
- 요청에 따라 일반적인 VR 배치와 반대로 좌 스틱은 회전, 우 스틱은 이동으로 지정한다.
- HMD가 담당하는 Pitch는 조이스틱으로 변경하지 않고 Yaw만 Snap Turn한다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공.
- `SuwonSiegeContestVR.Core.VR.InputAndHandAnimationConfiguration` 자동화 테스트 성공.
- Quest 좌/우 Thumbstick 키 등록과 `PoseAlphaGrasp` 실수형 속성 존재 확인.
- `CompileAllBlueprints`: 오류 0, 경고 0, 로드 실패 0.

# 잔여 문제

- Quest 3 실기에서 이동 방향, 멀미 민감도, 손 포즈 전환 체감은 재패키징 후 확인해야 한다.
