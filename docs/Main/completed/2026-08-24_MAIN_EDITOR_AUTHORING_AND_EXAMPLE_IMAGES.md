# 작업

Main 플레이 흐름 Editor Authoring 구조, 인터랙션 가이드, 예시 이미지 구현

# 구현 내용

- `DA_Scenario_MainEducation`에 `01 Editor Flow` Stage→Step 편집 트리를 추가했다.
- Stage/Step 배열 순서에서 Runtime Start/Next ID를 자동 생성한다.
- Step 하나에서 화면 데이터, 이미지, 퀴즈, 나레이션 Row, 체험 Route, 플레이어 행동, 완료 조건, 가이드, 지연 시간을 편집한다.
- Editor Flow 변경 시 자동 Rebuild하며 수동 `Rebuild Scenario From Editor Flow` 버튼도 제공한다.
- UI 가이드 문구를 C++ 하드코딩보다 Step의 `InteractionGuideText`에서 우선 읽도록 변경했다.
- 플레이어 행동·완료 조건·가이드 누락 및 Editor/Runtime 불일치를 검증한다.
- built-in `image_gen`으로 수원화성 교육용 예시 이미지 5종을 생성했다.
- PNG와 Texture를 `/Game/Art/MainEducation/Examples`에 저장하고 12개 Content 이미지 슬롯에 연결했다.
- 기존 최종 이미지가 있으면 구성 스크립트가 덮어쓰지 않도록 했다.

# 변경 파일

- `Source/SuwonSiegeContestVR/Public/Main/Education/MainEducationTypes.h`
- `Source/SuwonSiegeContestVR/Public/Main/Education/MainEducationScenarioDefinition.h`
- `Source/SuwonSiegeContestVR/Private/Main/Education/MainEducationScenarioDefinition.cpp`
- `Source/SuwonSiegeContestVR/Private/Main/Education/MainEducationScenarioManagerActor.cpp`
- `Source/SuwonSiegeContestVR/Private/Tests/MainEducationFlowTests.cpp`
- `Content/Art/MainEducation/Examples/*`
- `Content/Data/DA_Scenario_MainEducation.uasset`
- `Scripts/ConfigureMainEducationEditorAuthoring.py`
- `Scripts/VerifyMainEducationEditorAuthoring.py`
- `docs/Main/specs/MAIN_EDUCATION_FLOW.md`
- `docs/Main/STATUS.md`
- `docs/ARCHITECTURE.md`
- `docs/DIRECTORY_STRUCTURE.md`

# 주요 결정 사항

- 기존 Core Scenario 구조를 바꾸지 않고 Main 전용 Editor Flow에서 Runtime 구조를 생성한다.
- UI용 Step에는 `Content.InteractionGuideText`, 실제 월드 대상이 있는 경우에는 `WorldGuideAction/Text`를 사용한다.
- 예시 이미지 안에는 텍스트나 화살표를 넣지 않고 Widget Callout으로 분리했다.
- AI 생성 이미지는 편집·배치 확인용이며 최종 문화재 고증 이미지가 아니다.

# 테스트 결과

- UE 5.8 `SuwonSiegeContestVREditor Win64 Development` 빌드 성공.
- Editor 검증 성공: 6 Stage, 42 Step, 12 Image Slot, 5 Texture.
- Runtime Next ID와 Editor Flow 배열 순서 일치 확인.
- 기존 Main 나레이션 검증 성공: 33 Row, 33 SoundWave, 14 Segment.
- `Suwon.Main.Education.DefaultFlow` Automation Test 성공.

# 남은 문제

- AI 예시 이미지 5종은 문화재·복식·무기·도르래 구조 고증 후 최종 이미지로 교체해야 한다.
- 최종 Main 교육 Widget에서 Image, Callouts, HighlightText, InitialConsonants를 실제 레이아웃으로 표시해야 한다.
- 녹로·거중기 Experience/Level 연결과 옹성 완료 보고가 남아 있다.
- Android/VR 실기에서 이미지 해상도, Texture 메모리, 가이드 가독성을 확인해야 한다.
