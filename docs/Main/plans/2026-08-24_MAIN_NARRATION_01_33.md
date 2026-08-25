# 목적

**상태: 완료 (2026-08-24)**

`/Game/Audio/Narration`에 추가된 메인 레벨 나레이션 01~33번을 전용 Data Table에 등록하고 `DA_Scenario_MainEducation` 진행에 연결한다.

# 현재 상태

- 01~33번 SoundWave가 존재하며 녹로 원리 설명까지 녹음되어 있다.
- Main 교육 Scenario는 표시/퀴즈/체험 흐름만 있고 `NarrationTable`이 비어 있다.
- 기존 `/Game/Data/DT_Narration`은 신기전에서 사용 중이므로 Main 음원을 섞으면 안 된다.

# 구현 범위

- `/Game/Audio/Narration/DT_Narration_Main` 생성 및 33개 Row 등록
- 교육 화면별 Row 구간 연결과 마지막 Row `Stop` 설정
- Main Scenario의 해당 표시 단계를 `Narration` Interaction으로 변경
- `DA_Scenario_MainEducation.NarrationTable` 연결
- 퀴즈 질문 재생 완료 후 기존 음성 인식 포트 호출 유지

# 변경 예정 파일

- `Source/SuwonSiegeContestVR/Private/Main/Education/MainEducationScenarioDefinition.cpp`
- `Source/SuwonSiegeContestVR/Private/Main/Education/MainEducationScenarioManagerActor.cpp`
- `Scripts/CreateMainNarrationData.py`
- `Scripts/VerifyMainNarrationData.py`
- `Content/Audio/Narration/DT_Narration_Main.uasset`
- `Content/Data/DA_Scenario_MainEducation.uasset`
- Main 문서

# 구현 단계

1. 01~33번 자막과 교육 화면별 재생 구간을 정의한다.
2. Main 기본 Scenario의 해당 Interaction에 시작 Row를 지정한다.
3. Editor Python으로 DT를 생성하고 Main Scenario에 연결한다.
4. 빌드, Automation Test, Editor Asset 검증을 수행한다.

# 다른 Feature에 미치는 영향

- 신기전·공심돈 등 기존 체험 DT와 Scenario는 수정하지 않는다.
- Core Narration API는 변경하지 않는다.

# 검증 방법

- 33개 음원/Row의 번호 일치 확인
- 모든 Row가 한 교육 화면의 재생 구간 안에서만 연결되는지 확인
- Main Scenario가 전용 DT를 참조하고 모든 NarrationID가 존재하는지 확인
- UE 5.8 Editor 빌드 및 Main Automation Test 실행
