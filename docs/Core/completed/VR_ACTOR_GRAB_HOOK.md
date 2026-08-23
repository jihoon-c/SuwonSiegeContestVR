# 작업

Core VR Pawn의 범용 Actor Grab/Release 알림 추가

# 구현 내용

- 기존 `BP_GrabComponent` 탐색과 함께 `VRGrab` 태그가 붙은 Scene Component를 탐색한다.
- Grab 성공 시 소유 Actor의 선택적 `HandleVRGrabbed(GrabComponent, MotionController)`를 호출한다.
- Actor가 `false`를 반환하면 Grab을 거부하며, Release 시 선택적 `HandleVRReleased`를 호출한다.
- Core는 특정 Game Feature 클래스를 참조하지 않는다.

# 변경 파일

- `Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`

# 주요 결정 사항

Feature 전용 Blueprint 그래프나 클래스에 Core가 의존하지 않도록 컴포넌트 태그와 선택적 UFunction 계약을 사용했다.

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- GF_Singijeon 화차 자동화와 에셋 검증 성공

# 남은 문제

- Quest 3 VR Preview에서 실제 컨트롤러 Grab 반경 체감을 최종 확인한다.
