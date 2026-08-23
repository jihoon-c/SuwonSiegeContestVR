# 목적

**상태: 완료 (2026-08-21)**

Quest 왼쪽 Trigger 입력이 `BP_VRPlayerPawn`의 왼손 Grab으로 전달되지 않는 문제를 수정한다.

# 현재 상태

Pawn의 좌우 Grab 바인딩과 Motion Source는 정상이나 `IMC_Hands`의 실제 키 매핑이 비어 있다.

# 구현 범위

- `IMC_Hands`를 Pawn이 명시적으로 등록
- 좌우 Quest Trigger Axis 매핑 복구
- 매핑 유효성 자동화 테스트 추가

# 변경 예정 파일

- `VRPlayerPawn.h/.cpp`
- `ScenarioFrameworkTests.cpp`
- `/Game/XRFramework/Input/IMC_Hands`
- 입력 구성·검증 스크립트

# 구현 단계

1. Pawn에 Hand Mapping Context 참조 및 등록 추가
2. `IMC_Hands`에 좌우 Trigger 매핑 작성
3. 빌드 및 에디터 자동화 테스트

# 다른 Feature에 미치는 영향

Core VR 입력 변경이며 모든 체험 레벨의 `BP_VRPlayerPawn`에 동일하게 적용된다.

# 검증 방법

에셋 매핑 검사, C++ 빌드, Core VR 자동화 테스트, Quest VR Preview 재시작 후 양손 Trigger Grab 확인.
