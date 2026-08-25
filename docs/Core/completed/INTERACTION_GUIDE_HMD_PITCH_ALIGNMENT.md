# 작업

도화선처럼 HMD보다 낮고 가까운 대상의 인터랙션 가이드가 시점에 비스듬하게 보이는 문제를 보완했다.

# 구현 내용

- 가이드 회전에서 강제로 제거하던 Pitch를 유지한다.
- 활성 HMD Camera 기준 Yaw/Pitch를 30Hz로 따라가며 Roll만 0으로 고정한다.
- 공통 가이드 회전 자동화 테스트에 45도 Pitch와 Roll 고정을 검증한다.

# 변경 파일

- `ScenarioInteractionGuideComponent.cpp`
- `ScenarioFrameworkTests.cpp`
- `docs/Main/specs/SCENARIO_SYSTEM.md`

# 주요 결정 사항

별도 도화선 전용 Tick을 추가하지 않고 기존 공통 활성 가이드 하나의 30Hz 갱신을 사용한다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공
- `SuwonSiegeContestVR.Core.Scenario` 자동화 테스트 6개 성공
- `InteractionGuidePresentation`에서 Pitch 45도와 Roll 0도 검증 성공

# 남은 문제

Quest 3 HMD에서 도화선 접촉 자세의 최종 시인성을 확인한다.
