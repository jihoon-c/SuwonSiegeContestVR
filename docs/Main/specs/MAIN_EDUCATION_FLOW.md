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

현재 연결된 체험은 신기전과 옹성 두 개다. 두 체험 모두 같은 형태를 따른다.

```text
성문 앞 인사
→ 신기전 설명 → 초성 퀴즈 "ㅎ ㅊ"(화차) → LV_Singijeon 체험 → L_Main 복귀
→ 옹성 설명   → 초성 퀴즈 "ㅇ ㅅ"(옹성) → LV_Ongseong 체험 → L_Main 복귀
→ 전체 정리
```

| Stage | Step |
|---|---|
| `MAIN_GATE` | `GATE_GREETING` (Narration `MAIN_NA_01`) |
| `SINGIJEON` | `SINGIJEON_IMAGE` → `SINGIJEON_NARRATION`(`MAIN_NA_13`) → `SINGIJEON_QUIZ`(`QUIZ_HWACHA`) → `TRAVEL_SINGIJEON` → `AFTER_SINGIJEON` |
| `ONGSEONG` | `ONGSEONG_IMAGE` → `ONGSEONG_NARRATION`(`MAIN_NA_18`) → `ONGSEONG_QUIZ`(`QUIZ_ONGSEONG`) → `TRAVEL_ONGSEONG` → `AFTER_ONGSEONG` |
| `SUMMARY` | `SUMMARY_01` → `SUMMARY_NARRATION`(`MAIN_NA_25`) |

**퀴즈는 설명 나레이션이 끝난 뒤에만 열린다.** 질문 음성과 마이크가 겹치지 않게 하기 위한 배치다.

**Travel Step 뒤에는 반드시 Step이 하나 더 있어야 한다.** `BeginExperienceTravel()`이
`NextInteractionID`를 복귀 체크포인트로 저장하므로, Travel이 Stage의 마지막 Step이면 이동이 거부된다.
`AFTER_*`가 그 자리이자 체험을 마치고 돌아오는 지점이다.

**체험이 끝나는 조건**

| 체험 | 복귀 조건 |
|---|---|
| 신기전 | `DA_Scenario_Singijeon` 종료 시 `ScenarioExperienceBridgeComponent`가 복귀시킨다 |
| 옹성 | 충차 파괴(성공)로 복귀하거나, **전투 시작 30초 뒤 자동 복귀**(`BattleTimeLimit`) |

복귀 체크포인트가 남아 있으면 `AMainLevelIntroActor`는 성문 인트로 연출을 건너뛰고
곧바로 교육 흐름을 이어 간다(`bSkipIntroOnExperienceReturn`).

거중기·녹로·공심돈 설명 구간은 2026-08-27에 흐름에서 제외했다. Level과 Experience 에셋은 그대로 있으며,
Stage와 Route를 다시 추가하면 복원된다.

## 메인 레벨 시작 연출

`L_Main`의 `MainLevelIntro_CastleToGate`가 시작 연출을 담당한다. Level Blueprint가 아닌
`AMainLevelIntroActor`이므로 다른 Main 변형 Level에도 배치해 재사용할 수 있다.

```text
OverviewAnchor → TitleAnchor → (타이틀 3초) → 페이드 아웃 → PlayerAnchor → 교관 나레이션
```

이 연출은 **처음 입장할 때만** 재생된다. 체험에서 돌아와 복귀 체크포인트가 남아 있으면
연출을 건너뛰고 교육 흐름을 이어 간다(`bSkipIntroOnExperienceReturn`, 기본 켜짐).

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

현재 등록 범위는 `MAIN_NA_01` ~ `MAIN_NA_30`이다. Row는 `NextRow` + `AdvanceMode = Auto`로 이어지고
구간의 마지막 Row가 `Stop`이므로, Scenario가 지정한 시작 Row 하나로 구간 전체가 재생된다.

```text
01     건설 장치 예고
02~04  거중기
05~07  녹로 + 감시·방어 예고
08~12  공심돈
13~17  신기전  (16번에 "여러 발의 신기전을 화차에 장착하면 한꺼번에 발사")
18~24  옹성    (24번 "이번에는 … 성문을 방어하는 상황을 직접 체험해 보겠습니다")
25~30  마무리
```

현재 흐름이 쓰는 구간은 `01`, `13`, `18`, `25` 네 개다.

**퀴즈는 구간 재생이 끝난 뒤에 열린다.** 나레이션 Step이 완료되어야 다음 Quiz Step이 시작되고,
그때 `RequestVoiceRecognition`이 호출되어 마이크가 열린다. 질문 음성과 마이크는 겹치지 않는다.

흐름에서 빠진 거중기·녹로·공심돈을 `01`과 `26`이 여전히 언급한다. 음원 재녹음 또는 구간 재조정이
필요하며, 화면 텍스트는 실제 체험 내용에 맞게 이미 수정되어 있다.

## UI 연결

`AMainEducationScenarioManagerActor.OnEducationContentRequested`에 메인 VR Widget을 바인딩한다.
이벤트의 `FMainEducationContent`에는 제목, 본문, 핵심 문구, 이미지, Callout, 초성과 정답 데이터가 있다.

- 일반 설명/이미지 화면의 확인 입력 → `ContinuePresentation()`
- 퀴즈의 버튼/키보드/음성 인식 결과 → `SubmitQuizAnswer(Answer)`
- 정답/오답 UI → `OnQuizFeedback`
- 연결되지 않은 체험 안내 → `OnExperienceUnavailable`
- 개발 중 미구현 체험 통과 → `SkipUnavailableExperience()`

공통 VR HUD Component가 Pawn에 있으면 제목·본문과 각 Step의 `InteractionGuideText`가 기본 미러링된다.
이미지의 최종 레이아웃은 별도 Main Widget에서 위 이벤트를 소비한다.
퀴즈 화면은 Core 퀴즈 패널이 담당하며, 그동안 Main 프레젠테이션 패널은 숨는다.

## 음성 퀴즈 연결 (2026-08-27 구현)

퀴즈 Step은 음성으로 답한다. Manager가 Core 초성 퀴즈 런타임을 구동한다.

```text
Quiz Interaction
      ↓
AMainEducationScenarioManagerActor::RequestVoiceRecognition(QuizID)
      ↓  FMainEducationContent(질문·초성·정답) → FInitialConsonantQuizDefinition
UInitialConsonantQuizComponent (Core) — 패널 · 시도 · 마이크 소유
      ↓  OnQuizFinished
정답      → SubmitQuizAnswer(정답)              → 다음 Step(Travel)
시도 소진 → 정답 공개 후 CompleteQuizInteraction → 다음 Step(Travel)
취소      → 아무 것도 하지 않는다 (레벨 종료·재시작)
```

* **퀴즈 데이터는 Main Content에만 둔다.** 별도의 Core 퀴즈 Data Asset을 만들지 않으므로
  디자이너는 지금까지처럼 `01 Editor Flow`의 Quiz Step만 편집하면 된다.
  `Initial Consonants`를 비워 두면 정답에서 자동으로 추출된다.
* 퀴즈 Step 동안 Main 프레젠테이션 패널은 숨고 Core 퀴즈 패널만 보인다.
* `SubmitQuizAnswer(Answer)`는 그대로 열려 있다. 버튼·키보드·콘솔로도 같은 판정 경로를 쓴다.
  마이크 없이 확인하려면 콘솔 `ssv.voice.submit 화차`.
* 조정 프로퍼티: `bUseVoiceQuiz`(기본 켜짐), `QuizListenDuration`(8초), `QuizMaxAttempts`(3회).
* 다른 음성 백엔드를 쓰려면 `RequestVoiceRecognition` / `CancelVoiceRecognition`를 오버라이드한다.

Core 런타임 상세는 `docs/Core/specs/INITIAL_CONSONANT_QUIZ.md`를 참조한다.

## 체험 연결 현황

| Route | 기본 연결 | 흐름 포함 |
|---|---|---|
| 신기전 | `DA_Experience_Singijeon` | 포함 (`Travel_Singijeon`) |
| 옹성 | `DA_Experience_Ongseong` | 포함 (`Travel_Ongseong`) |
| 공심돈 | `DA_Experience_Gongsimdon` | 미포함 — 에셋만 존재 |
| 녹로 | Level/Experience 미존재 | 미포함 |
| 거중기 | `DA_Experience_Geojunggi` 경로 예약, 현재 Asset 미존재 | 미포함 |

옹성 Level 자체의 초성 퀴즈(`bRunIntroQuiz`)는 Main이 같은 질문을 내므로 **기본 꺼짐**이다.

체험 완료 담당자는 해당 Level에서 `UExperienceSubsystem.CompleteCurrentExperience(true)`를 호출해야
Main 복귀 체크포인트가 복원된다.

## 생성

Editor Target 컴파일 후 `Scripts/CreateMainEducationFlow.py`를 Unreal Editor Python으로 실행한다.
스크립트는 `DA_Scenario_MainEducation`, Main/Ongseong Experience 연결, `L_Main`의 전용 Manager를 구성한다.

새 메인 음원을 등록하거나 Row 연결을 재생성할 때는 `Scripts/CreateMainNarrationData.py`를 실행하고,
`Scripts/VerifyMainNarrationData.py`로 SoundWave/DT/Scenario 연결을 확인한다.

예시 Texture를 다시 임포트하거나 빈 이미지 슬롯을 연결할 때는
`Scripts/ConfigureMainEducationEditorAuthoring.py`를 실행한다. 이미 최종 이미지가 지정된 슬롯은 덮어쓰지 않는다.
