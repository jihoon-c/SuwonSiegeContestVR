# 작업

메인 레벨 교육 시나리오 전체 흐름 베이스 구현

# 구현 내용

- Game Feature Plugin 바깥의 프로젝트 Runtime 모듈 `Main/Education`에 전용 구조를 추가했다.
- `UMainEducationScenarioDefinition` 기본값으로 6개 Stage, 37개 표시 콘텐츠, 4개 초성 퀴즈, 5개 체험 Route를 구성했다.
- 시나리오 순서를 신기전 → 공심돈 → 옹성 → 녹로 → 거중기 → 전체 정리로 구현했다.
- 교관 본문, 이미지 제목/핵심 문구/Callout, 초성과 정답을 Data Asset 편집 가능 데이터로 제공했다.
- `AMainEducationScenarioManagerActor`가 Core Scenario 진행, UI 이벤트, 퀴즈 정답 판정, Experience 이동/복귀 체크포인트를 중계한다.
- 공통 VR HUD가 있으면 텍스트를 기본 미러링하고, 최종 이미지/퀴즈 UI는 Delegate 기반으로 분리했다.
- 음성 인식은 빈 `RequestVoiceRecognition`/`CancelVoiceRecognition` Blueprint Native Event와 `SubmitQuizAnswer` 연결점만 제공했다.
- `DA_Scenario_MainEducation`, `DA_Experience_Ongseong`을 생성하고 `DA_Experience_Main`, `L_Main`을 전용 흐름에 연결했다.

# 변경 파일

- `Source/SuwonSiegeContestVR/Public/Main/Education/`
- `Source/SuwonSiegeContestVR/Private/Main/Education/`
- `Source/SuwonSiegeContestVR/Private/Tests/MainEducationFlowTests.cpp`
- `Content/Data/DA_Scenario_MainEducation.uasset`
- `Content/Core/Experience/Definitions/DA_Experience_Main.uasset`
- `Content/Core/Experience/Definitions/DA_Experience_Ongseong.uasset`
- `Content/Maps/Main/L_Main.umap`
- `Scripts/CreateMainEducationFlow.py`
- `Scripts/VerifyMainEducationFlow.py`
- `docs/Main/specs/MAIN_EDUCATION_FLOW.md`
- `docs/Main/STATUS.md`
- `docs/ARCHITECTURE.md`
- `docs/DIRECTORY_STRUCTURE.md`

# 주요 결정 사항

- 기존 `UScenarioManagerComponent`와 `UExperienceSubsystem`의 공개 API는 변경하지 않았다.
- Main은 `UExperienceDefinition` Soft Reference만 사용하며 GF C++ 모듈에 직접 의존하지 않는다.
- 이미지 에셋이 제공되지 않았으므로 Texture 슬롯은 비워 두고 화면용 메타데이터만 구현했다.
- STT 소유권은 별도 담당자에게 유지하고, 정답 판정 진입점만 안정적인 계약으로 제공했다.
- 없는 체험을 가짜 Level로 만들지 않고 `OnExperienceUnavailable`과 개발용 Skip으로 명시했다.

# 테스트 결과

- UE 5.8 `SuwonSiegeContestVREditor Win64 Development` 빌드 성공.
- Automation `Suwon.Main.Education.DefaultFlow` 성공.
  - Core Scenario 및 Main 콘텐츠 참조 유효성
  - 6개 Stage / 5개 Route
  - 공심돈·옹성·녹로·거중기 정답 판정
  - 정답 문자열의 공백 정규화
- `Scripts/VerifyMainEducationFlow.py` 성공.
  - 6 Stage, 37 Content, 4 Quiz, 5 Route
  - `DA_Experience_Main` Scenario 연결
  - `L_Main` 전용 Manager 1개 배치

# 남은 문제

- 37개 콘텐츠의 최종 Texture와 전용 VR 교육 Widget을 제작·연결해야 한다.
- 녹로 Experience/Level은 아직 없다.
- 거중기 경로는 예약되어 있으나 Experience/Level Asset이 아직 없다.
- 옹성 Level의 실제 완료 조건에서 `CompleteCurrentExperience(true)` 호출을 연결해야 한다.
- 음성 인식 담당자가 빈 Blueprint Native Event를 구현하고 인식 결과를 `SubmitQuizAnswer`로 전달해야 한다.
- Android SDK 상태가 현재 검증 환경에서 invalid로 표시되어 Android 패키징/실기 검증은 수행하지 못했다.
