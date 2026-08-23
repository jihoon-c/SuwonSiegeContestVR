# 작업

Quest 왼쪽 Trigger Grab 입력 복구

# 구현 내용

- 비어 있던 `IMC_Hands`에 좌우 Quest Trigger Axis 매핑을 저장했다.
- `AVRPlayerPawn`이 `IMC_Hands`를 명시적으로 등록하도록 보완했다.
- 좌우 Action과 Key의 실제 매핑 쌍을 검사하는 자동화 테스트를 추가했다.

# 변경 파일

- `VRPlayerPawn.h/.cpp`
- `ScenarioFrameworkTests.cpp`
- `/Game/XRFramework/Input/IMC_Hands`
- `Scripts/ConfigureVRHandGrabInput.py`
- `Scripts/VerifyVRHandGrabInput.py`

# 주요 결정 사항

런타임 임시 Action 대신 OpenXR 시작 시 등록되는 정적 Input Action과 Mapping Context를 사용한다.

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- 좌우 Quest Trigger 매핑 에셋 검증 성공
- 전체 자동화 테스트 14/14 성공

# 남은 문제

에디터 재시작 후 Quest 3 VR Preview에서 실제 컨트롤러 입력을 최종 확인해야 한다.
