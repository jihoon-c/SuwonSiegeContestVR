# 목적

**상태: 완료 (2026-08-24)**

메인 교육의 플레이 흐름, 화면 이미지, 플레이어 행동, 완료 조건, 인터랙션 가이드를 `DA_Scenario_MainEducation` 하나에서 쉽게 편집하도록 개선한다.

# 현재 상태

- Runtime `Stages`와 `EducationContent`가 별도 배열이라 ID를 맞춰 편집해야 한다.
- 이미지 슬롯은 있으나 예시 Texture가 없다.
- UI 가이드 문구가 Manager에 하드코딩되어 있다.
- 실제 흐름을 이해하려면 C++ 기본값과 Scenario 배열을 함께 확인해야 한다.

# 구현 범위

- Stage → Step 계층의 `Editor Flow` 단일 편집 구조 추가
- Step 순서에서 Runtime Scenario 및 다음 Step 자동 생성
- Step별 화면 데이터, 나레이션 시작 Row, Route, 플레이어 행동, 완료 조건, 가이드, 메모 편집
- Editor Flow 변경 시 Runtime 데이터 자동 재생성 및 수동 Rebuild 버튼 제공
- 수원화성 교육용 예시 이미지 5종 생성·임포트·기본 슬롯 연결
- 이미지 교체와 흐름 편집 절차 문서화

# 변경 예정 파일

- `Source/SuwonSiegeContestVR/Public/Main/Education/*`
- `Source/SuwonSiegeContestVR/Private/Main/Education/*`
- `Scripts/ConfigureMainEducationEditorAuthoring.py`
- `Scripts/VerifyMainEducationEditorAuthoring.py`
- `Content/Art/MainEducation/Examples/*`
- `Content/Data/DA_Scenario_MainEducation.uasset`
- Main 문서 및 자동화 테스트

# 구현 단계

1. Editor Flow용 Stage/Step 구조를 추가한다.
2. 기존 기본 Scenario에서 Editor Flow를 생성하고 역으로 Runtime Scenario를 재생성한다.
3. UI 가이드 문구를 Step 데이터에서 소비한다.
4. 예시 PNG를 프로젝트에 보관하고 Texture로 임포트한다.
5. 기본 이미지 슬롯을 연결하고 Editor/Automation 검증을 수행한다.

# 다른 Feature에 미치는 영향

- Core Scenario/Experience/Narration API는 변경하지 않는다.
- Game Feature의 Level과 콘텐츠는 수정하지 않는다.
- Main 전용 Asset과 Runtime 클래스만 확장한다.

# 검증 방법

- Editor Flow 6 Stage와 Runtime 6 Stage의 순서/Step 수 일치
- Runtime 모든 Interaction의 Next ID 자동 연결 확인
- 5개 예시 Texture 및 지정된 콘텐츠 이미지 슬롯 확인
- Main Scenario/Narration 기존 검증 재실행
- UE 5.8 Editor 빌드와 Automation Test 실행
