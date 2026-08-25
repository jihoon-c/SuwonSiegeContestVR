# Main Education Flow

## 역할과 위치

메인 교육 흐름은 Game Feature Plugin이 아니라 프로젝트 Runtime 모듈의
`Source/SuwonSiegeContestVR/Main/Education`에 둔다. 개별 체험 구현은 계속 `GF_*`가 소유하고,
Main은 Core `UExperienceDefinition` Data Asset을 통해서만 체험을 호출한다.

```text
AMainEducationScenarioManagerActor
  → UMainEducationScenarioDefinition (전체 교육 데이터)
  → UScenarioManagerComponent (레벨 내부 진행)
  → UExperienceSubsystem (체험 레벨 이동/복귀)
```

## 기본 흐름

```text
부임·전체 구조·성벽 방어
→ 신기전 체험
→ 공심돈 상황/퀴즈/설명/체험
→ 옹성 상황/퀴즈/설명/체험
→ 녹로 상황/퀴즈/설명/체험
→ 거중기 상황/퀴즈/설명/비교/체험
→ 전체 정리
```

## 메인 레벨 시작 연출

`L_Main`의 `MainLevelIntro_CastleToGate`가 시작 연출을 담당한다. Level Blueprint가 아닌
`AMainLevelIntroActor`이므로 다른 Main 변형 Level에도 배치해 재사용할 수 있다.

```text
OverviewAnchor → TitleAnchor → (타이틀 3초) → 페이드 아웃 → PlayerAnchor → 교관 나레이션
```

Actor의 파란 `OverviewAnchor`, 노란 `TitleAnchor`, 초록 `PlayerAnchor` 화살표를 Viewport에서
직접 이동·회전해 세 시점을 잡는다. 기본 설정에서 `TitleAnchor`와 `PlayerAnchor`는 같은 정문
플레이어 시점이며, 전경에서 해당 시점까지 5초 동안 lerp한다. 타이틀을 별도 구도에서 보여주려면
`TitleAnchor`를 옮기고 `Title To Player Duration`을 설정한다.

`AMainEducationScenarioManagerActor.bWaitForIntroSequence`가 켜진 경우에만 Intro Actor가
`StartEducationAfterIntro()`로 첫 교관 나레이션을 시작한다. Intro Actor를 제거하는 Level에서는
이 값을 꺼서 기존 자동 시작을 사용한다.

기본 교관 문구, 이미지 설명 슬롯, 핵심 문구, Callout, 초성과 정답은
`UMainEducationScenarioDefinition`의 기본값으로 제공한다. 최종 교육 이미지가 준비되면 아래
`Editor Flow`의 해당 Step에서 Texture를 교체한다.

## 에디터에서 흐름 편집

`DA_Scenario_MainEducation`을 열고 `01 Editor Flow`만 편집한다.

```text
01 Editor Flow
└─ Stage (배열 순서 = Stage 진행 순서)
   └─ Steps (배열 순서 = Step 진행 순서)
      ├─ Step ID
      ├─ Step Type
      ├─ Content: 제목/본문/이미지/퀴즈/가이드/Editor Notes
      ├─ Narration Start Row 또는 Experience Route ID
      ├─ Player Action
      ├─ Completion Condition
      ├─ World Guide Action/Text
      └─ Delay Before/After
```

- Step을 위아래로 옮기면 `NextInteractionID`는 배열 순서에서 자동 생성된다.
- Stage를 옮기면 다음 Stage도 배열 순서에서 자동 생성된다.
- `bAutoRebuildFromEditorFlow`가 켜져 있으면 편집 즉시 Runtime `Stages`와 `EducationContent`가 갱신된다.
- 자동 갱신을 꺼 두었으면 `Rebuild Scenario From Editor Flow` 버튼을 누른다.
- `Generated Runtime`과 상속된 Core Scenario 배열은 확인용이며 직접 편집하지 않는다.
- `Player Action`, `Completion Condition`, `Interaction Guide Text`가 비어 있으면 검증에 실패한다.

Step Type의 의미:

| Step Type | 진행 방식 |
|---|---|
| Narration + Presentation | 지정 Row 구간 재생 완료 시 다음 Step |
| Presentation / Confirm | 이미지·설명 확인 후 `ContinuePresentation()` |
| Quiz / Answer | 정답이 Accepted Answers와 일치하면 다음 Step |
| Experience Travel | Route의 체험으로 이동하고 완료 후 다음 Step 복귀 |

## 예시 이미지

`/Game/Art/MainEducation/Examples`에 교체 가능한 예시 Texture 5종을 제공한다.

| 예시 | 연결 화면 |
|---|---|
| 수원화성 조감도 | 전체 구조, 최종 정리 |
| 성벽 방어 | 화약무기 안내 |
| 공심돈 단면 | 공심돈 실제 모습·단면 |
| 옹성 평면 | 옹성 구조·유무 비교 |
| 녹로·거중기 비교 | 녹로/거중기 구조 및 비교 |

총 12개 Content 이미지 슬롯에 연결되어 있다. 예시는 AI 생성 이미지이므로 최종 납품 전 문화재·복식·무기·기계
고증을 거쳐 교체한다. 이미지 안에는 텍스트를 넣지 않았으며 Callout과 핵심 문구는 Widget에서 표시한다.

## 메인 나레이션

메인 전용 나레이션은 `/Game/Audio/Narration/DT_Narration_Main`에서 관리한다. 기존 신기전
`/Game/Data/DT_Narration`과 분리되어 있어 Main 음원 추가가 체험 나레이션 순서에 영향을 주지 않는다.

현재 등록 범위는 01~33번이며 녹로 원리 설명까지다. Scenario는 다음 14개 화면 단위로 재생한다.

```text
01~03 부임
04~07 성벽 방어
08 신기전 체험 전환
09~11 공심돈 상황·질문
12~13 공심돈 정답
14 공심돈 이미지 설명
15~16 공심돈 체험 전환
17~20 옹성 상황·질문
21~22 옹성 정답
23~24 옹성 비교 설명
25~26 옹성 체험 전환
27~30 녹로 상황·질문
31~32 녹로 정답
33 녹로 원리 설명
```

각 구간 마지막 Row는 `Stop`이다. 질문 음성이 끝난 뒤 Scenario가 Quiz Interaction으로 이동하므로,
음성 인식 담당자의 `RequestVoiceRecognition`도 질문 재생 완료 후 호출된다. 거중기와 최종 정리는
해당 음원이 추가될 때 별도 구간을 이어서 등록한다.

## UI 연결

`AMainEducationScenarioManagerActor.OnEducationContentRequested`에 메인 VR Widget을 바인딩한다.
이벤트의 `FMainEducationContent`에는 제목, 본문, 핵심 문구, 이미지, Callout, 초성과 정답 데이터가 있다.

- 일반 설명/이미지 화면의 확인 입력 → `ContinuePresentation()`
- 퀴즈의 버튼/키보드/음성 인식 결과 → `SubmitQuizAnswer(Answer)`
- 정답/오답 UI → `OnQuizFeedback`
- 연결되지 않은 체험 안내 → `OnExperienceUnavailable`
- 개발 중 미구현 체험 통과 → `SkipUnavailableExperience()`

공통 VR HUD Component가 Pawn에 있으면 제목·본문과 각 Step의 `InteractionGuideText`가 기본 미러링된다.
이미지와 퀴즈의 최종 레이아웃은 별도 Main Widget에서 위 이벤트를 소비한다.

## 음성 인식 담당자 연결점

이 베이스에는 마이크 캡처, 권한 요청, 음성 모델, STT 판정 코드가 없다.

1. `RequestVoiceRecognition(QuizID)`를 Blueprint 또는 별도 모듈에서 구현해 녹음을 시작한다.
2. 인식된 최종 문자열을 `SubmitQuizAnswer(RecognizedText)`에 전달한다.
3. `CancelVoiceRecognition()`에서 녹음/인식을 중지한다.

두 Blueprint Native Event의 C++ 기본 구현은 의도적으로 비어 있다. 따라서 음성 모듈이 없어도
버튼 또는 개발용 텍스트 입력으로 같은 정답 판정 경로를 검증할 수 있다.

## 체험 연결 현황

| Route | 기본 연결 |
|---|---|
| 신기전 | `DA_Experience_Singijeon` |
| 공심돈 | `DA_Experience_Gongsimdon` |
| 옹성 | 생성 스크립트가 `DA_Experience_Ongseong`을 구성 |
| 녹로 | Level/Experience 미존재, 빈 슬롯 |
| 거중기 | `DA_Experience_Geojunggi` 경로 예약, 현재 Asset 미존재 |

체험 완료 담당자는 해당 Level에서 `UExperienceSubsystem.CompleteCurrentExperience(true)`를 호출해야
Main 복귀 체크포인트가 복원된다.

## 생성

Editor Target 컴파일 후 `Scripts/CreateMainEducationFlow.py`를 Unreal Editor Python으로 실행한다.
스크립트는 `DA_Scenario_MainEducation`, Main/Ongseong Experience 연결, `L_Main`의 전용 Manager를 구성한다.

새 메인 음원을 등록하거나 Row 연결을 재생성할 때는 `Scripts/CreateMainNarrationData.py`를 실행하고,
`Scripts/VerifyMainNarrationData.py`로 SoundWave/DT/Scenario 연결을 확인한다.

예시 Texture를 다시 임포트하거나 빈 이미지 슬롯을 연결할 때는
`Scripts/ConfigureMainEducationEditorAuthoring.py`를 실행한다. 이미 최종 이미지가 지정된 슬롯은 덮어쓰지 않는다.
