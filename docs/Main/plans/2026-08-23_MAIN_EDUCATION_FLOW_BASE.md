# 목적

**상태: 완료 (2026-08-23)**

수원화성 메인 레벨 교육 시나리오 전체 흐름을 Game Feature Plugin이 아닌 게임 프로젝트 Runtime 모듈에 구현한다.

# 현재 상태

- Core Scenario/Experience/Narration 기반과 `L_Main`, `DA_Experience_Main`이 존재한다.
- 현재 Main Scenario Asset은 공심돈과 신기전 왕복 예시만 포함한다.
- 초성 퀴즈, 설명 이미지 요청, 옹성·녹로·거중기 및 최종 정리 흐름은 구현되어 있지 않다.
- 음성 인식 모듈은 다른 담당자가 구현할 예정이며 현재 프로젝트에는 SDK가 없다.

# 구현 범위

- `Source/SuwonSiegeContestVR/Main/Education`에 메인 레벨 전용 데이터와 Manager Actor를 추가한다.
- 첨부 시나리오의 교육 순서, 교관 문구, 이미지 슬롯, 초성 퀴즈 정답 데이터를 기본값으로 제공한다.
- 기존 `UScenarioManagerComponent`와 `UExperienceSubsystem`을 사용해 진행 및 체험 왕복을 연결한다.
- 이미지/UI는 Blueprint가 소비할 수 있는 이벤트와 빈 Texture 슬롯로 제공한다.
- 음성 인식은 시작/중지 Blueprint 이벤트와 정답 제출 진입점만 제공하고 캡처·STT를 구현하지 않는다.
- 아직 Level/Experience가 없는 녹로·거중기 등은 명시적인 미연결 상태와 개발용 Skip API를 제공한다.

# 변경 예정 파일

- `Source/SuwonSiegeContestVR/Public/Main/Education/*`
- `Source/SuwonSiegeContestVR/Private/Main/Education/*`
- `Source/SuwonSiegeContestVR/Private/Tests/MainEducationFlowTests.cpp`
- `Scripts/CreateMainEducationFlow.py`
- `docs/Main/specs/MAIN_EDUCATION_FLOW.md`
- `docs/Main/STATUS.md`
- `docs/DIRECTORY_STRUCTURE.md`

# 구현 단계

1. 교육 콘텐츠/퀴즈/체험 경로 데이터 타입을 정의한다.
2. 기본 Scenario Definition에 전체 시나리오를 구성한다.
3. Main 전용 Manager Actor가 UI, 퀴즈, 음성 인식 포트, Experience 전환을 중계하게 한다.
4. Editor Python으로 Data Asset과 Main Level 배치를 생성한다.
5. 자동화 테스트와 Win64 Development Editor 빌드로 검증한다.

# 다른 Feature에 미치는 영향

- Core Scenario/Experience 공개 API는 변경하지 않는다.
- Main은 구체 GF C++ 모듈을 참조하지 않으며 Experience Data Asset만 소비한다.
- 기존 체험 Level과 Feature 코드는 수정하지 않는다.

# 검증 방법

- 기본 Scenario 유효성 및 네 개 퀴즈의 정답 판정을 Automation Test로 확인한다.
- 프로젝트 Editor Target을 빌드한다.
- Editor Python 실행 후 생성 Asset, Main Manager 배치, 연결된/미연결 Experience 경로를 검사한다.
