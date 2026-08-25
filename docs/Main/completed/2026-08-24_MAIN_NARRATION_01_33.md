# 작업

메인 레벨 나레이션 01~33번 DT 등록 및 Scenario 연결

# 구현 내용

- `/Game/Audio/Narration`의 번호형 SoundWave 01~33을 검색·검증했다.
- 같은 폴더에 `DT_Narration_Main`을 생성하고 `MAIN_NA_01`~`MAIN_NA_33` Row를 등록했다.
- 원본 교육 문구를 자막으로 입력하고 각 Row에 번호가 같은 SoundWave를 연결했다.
- 33개 Row를 14개 교육 화면 재생 구간으로 분리하고 각 구간 마지막을 `Stop`으로 설정했다.
- `DA_Scenario_MainEducation.NarrationTable`에 전용 DT를 연결했다.
- Main 기본 Scenario의 부임·방어·신기전 전환·공심돈·옹성·녹로 관련 표시 단계를 Narration Interaction으로 변경했다.
- 나레이션 중 `ContinuePresentation()`이 단계를 조기 완료하지 않도록 막았다.
- 질문 구간 종료 후 Quiz가 시작되어 향후 STT 요청이 질문 음성과 겹치지 않게 했다.

# 변경 파일

- `Content/Audio/Narration/DT_Narration_Main.uasset`
- `Content/Data/DA_Scenario_MainEducation.uasset`
- `Source/SuwonSiegeContestVR/Private/Main/Education/MainEducationScenarioDefinition.cpp`
- `Source/SuwonSiegeContestVR/Private/Main/Education/MainEducationScenarioManagerActor.cpp`
- `Source/SuwonSiegeContestVR/Private/Tests/MainEducationFlowTests.cpp`
- `Scripts/CreateMainNarrationData.py`
- `Scripts/VerifyMainNarrationData.py`
- `docs/Main/specs/MAIN_EDUCATION_FLOW.md`
- `docs/Main/STATUS.md`
- `docs/ARCHITECTURE.md`

# 주요 결정 사항

- 신기전용 `/Game/Data/DT_Narration`을 수정하지 않고 Main 전용 DT를 사용했다.
- 여러 문장을 한 교육 화면에서 재생하되 다음 화면이나 퀴즈로 음성이 넘어가지 않도록 구간을 명시했다.
- 이미지/퀴즈 표시 이벤트는 유지하고 Scenario Narration Bridge만 재사용했다.

# 테스트 결과

- UE 5.8 `SuwonSiegeContestVREditor Win64 Development` 빌드 성공.
- `Suwon.Main.Education.DefaultFlow` Automation Test 성공.
- Editor Asset 검증 성공: 33 Row, 33 SoundWave, 14 Scenario Narration Segment.
- 모든 Scenario NarrationID가 `DT_Narration_Main`에 존재함을 확인했다.

# 남은 문제

- 현재 제공 음원이 33번까지이므로 거중기와 전체 학습 정리 구간은 텍스트 진행만 사용한다.
- 신규 음원 추가 시 자막과 재생 구간을 `CreateMainNarrationData.py`에 추가해야 한다.
- Android 패키징 및 VR 실기에서 실제 음량, 자막 타이밍, 공간감을 확인해야 한다.
