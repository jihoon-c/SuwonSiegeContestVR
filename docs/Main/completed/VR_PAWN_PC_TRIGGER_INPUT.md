# VR Pawn PC 및 Trigger 입력 보완

## 작업

패키징 없이 PC PIE에서 이동을 시험할 수 있게 하고, Quest 좌우 Grab 입력을 Trigger로 통일했다.

## 구현 내용

- PC PIE: `W/A/S/D` 이동, `Q/E` Snap Turn
- PC 손별 Grab 확인: `F` 왼손, `G` 오른손
- Quest 왼손 Grab: `OculusTouch_Left_Trigger_Axis`
- Quest 오른손 Grab: `OculusTouch_Right_Trigger_Axis`
- Trigger Started에서 Grab과 손 자세를 적용하고 Completed/Canceled에서 해제
- 기존 Grip Grab 액션 바인딩 제거
- `Enable Move`는 Quest 오른쪽 스틱과 PC `W/A/S/D` 이동을 함께 제어

## 변경 파일

- `Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`
- `Source/SuwonSiegeContestVR/Private/Tests/ScenarioFrameworkTests.cpp`

## 검증

- `SuwonSiegeContestVREditor Win64 Development` 코드 컴파일 성공
- 실행 중인 에디터용 Hot Reload 모듈 링크 성공
- 표준 `SuwonSiegeContestVREditor Win64 Development` DLL 재빌드 성공
- OpenXR 컨트롤러의 실제 축 값 수신은 Quest Link/Air Link의 VR Preview에서 확인 필요

## PC 연결 VR Preview 입력 수정

- Pawn 생성 후 만든 Transient Input Action은 OpenXR 세션의 컨트롤러 Action Set에 포함되지 않을 수 있었다.
- 오른쪽 스틱 이동은 시작 시 등록되는 `IMC_Menu`의 `IA_Menu_Cursor_Right`를 사용하도록 변경했다.
- `IMC_Menu`가 입력 모드 필터로 비활성화되는 경우를 대비해 `IMC_Default`의 `IA_Move`도 이동 입력으로 함께 바인딩했다.
- 왼쪽 스틱 회전은 시작 시 등록되는 `IMC_Default`의 `IA_Turn`을 사용한다.
- 좌우 Trigger Grab은 시작 시 등록되는 `IMC_Hands`의 `IA_Hand_IndexCurl_Left/Right`를 사용한다.
- 런타임 Mapping Context는 PC 키보드 보조 입력에만 사용한다.

## 에디터 적용 주의

에디터를 다시 실행한 뒤 `VR Preview`로 테스트해야 시작 시 OpenXR Action Set에 변경된 정적 액션 구성이 반영된다.
