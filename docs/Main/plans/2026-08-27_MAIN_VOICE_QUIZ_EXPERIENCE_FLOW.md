**상태**: `Status: Completed` (2026-08-27) —
결과 기록은 `docs/Main/completed/2026-08-27_MAIN_VOICE_QUIZ_EXPERIENCE_FLOW.md`

# 목적

Main 교육 흐름에 **음성 인식 초성 퀴즈**를 연결하고, 정답을 맞히면 해당 체험 Level로 이동하도록 만든다.

```text
신기전 설명 나레이션 종료
        ↓
초성 퀴즈 "ㅎ ㅊ" (정답: 화차)
        ↓
LV_Singijeon 체험
        ↓
L_Main 복귀
        ↓
옹성 설명 나레이션 종료
        ↓
초성 퀴즈 "ㅇ ㅅ" (정답: 옹성)
        ↓
LV_Ongseong 체험
        ↓
L_Main 복귀 → 교육 마무리
```

# 현재 상태

| 요소 | 상태 |
|---|---|
| Core 초성 퀴즈 런타임 (`UInitialConsonantQuizComponent`) | 구현 완료. 음성 인식(sherpa-onnx) · 월드 패널 · 시도 관리 포함 |
| Core 음성 인식 (`USherpaVoiceRecognitionComponent`) | 구현 완료 |
| Main 교육 흐름 (`DA_Scenario_MainEducation`) | 6 Stage 설명 전용. **퀴즈 Step 없음, ExperienceRoutes 비어 있음** |
| `AMainEducationScenarioManagerActor` | `RequestVoiceRecognition()` / `CancelVoiceRecognition()` 포트가 **빈 구현** |
| `Travel_*` Step → `UExperienceSubsystem` 이동 · 복귀 체크포인트 | 구현 완료 (`BeginExperienceTravel`) |
| `DA_Experience_Singijeon` / `DA_Experience_Ongseong` | 존재. 둘 다 `ReturnLevel = L_Main` |
| 옹성 레벨 자체 초성 퀴즈 (`bRunIntroQuiz`) | 기본 켜짐 — Main에서 물으면 중복 |

즉 **퀴즈 런타임**과 **레벨 왕복**은 이미 있고, 둘을 Main 흐름에 잇는 부분만 없다.

## 나레이션 구간 (`DT_Narration_Main`)

`nextRow` 체인 + `advanceMode=Stop` 으로 구간이 나뉜다.

| 구간 | 내용 |
|---|---|
| `MAIN_NA_13` ~ `17` | 신기전. 16번에 "여러 발의 신기전을 **화차**에 장착하면 한꺼번에 발사할 수 있었습니다" |
| `MAIN_NA_18` ~ `24` | 옹성. 24번 "이번에는 옹성의 구조를 이용해 성문을 방어하는 상황을 직접 체험해 보겠습니다" |

두 구간 모두 끝에서 퀴즈 → 체험 이동으로 이어지는 것이 자연스럽다.

# 구현 범위

## 사용자 확정 사항 (2026-08-27)

1. 옹성 레벨 자체 초성 퀴즈는 **끈다**. 옹성 퀴즈는 Main에서만 출제한다.
2. 오답 정책은 **3회 후 정답 공개 → 그대로 이동**. 교육 콘텐츠가 막히지 않는다.
3. 퀴즈 UI는 **Core 초성 퀴즈 패널**을 그대로 쓴다. Main 프레젠테이션 패널은 퀴즈 동안 숨긴다.
4. Main 흐름은 **신기전·옹성만** 남긴다. 거중기·녹로·공심돈 설명 Stage는 제거한다.
   성문 앞 인사(`MAIN_GATE`)와 교육 마무리(`SUMMARY`)는 흐름의 시작·끝이므로 유지한다.

## 새 Main 흐름

```text
MAIN_GATE   GATE_GREETING        Narration  MAIN_NA_01
SINGIJEON   SINGIJEON_IMAGE      Presentation
            SINGIJEON_NARRATION  Narration  MAIN_NA_13 ~ 17
            SINGIJEON_QUIZ       Quiz       QUIZ_HWACHA  (ㅎ ㅊ / 화차)
            TRAVEL_SINGIJEON     Travel     Travel_Singijeon
            AFTER_SINGIJEON      Presentation  ← 복귀 체크포인트
ONGSEONG    ONGSEONG_IMAGE       Presentation
            ONGSEONG_NARRATION   Narration  MAIN_NA_18 ~ 24
            ONGSEONG_QUIZ        Quiz       QUIZ_ONGSEONG (ㅇ ㅅ / 옹성)
            TRAVEL_ONGSEONG      Travel     Travel_Ongseong
            AFTER_ONGSEONG       Presentation  ← 복귀 체크포인트
SUMMARY     SUMMARY_01           Presentation
            SUMMARY_NARRATION    Narration  MAIN_NA_25 ~ 30
```

`Travel_*` Step 뒤에는 반드시 Step이 하나 더 있어야 한다.
`BeginExperienceTravel()`이 `NextInteractionID`를 복귀 체크포인트로 저장하기 때문에,
Travel이 Stage의 마지막 Step이면 이동 자체가 거부된다. `AFTER_*`가 그 자리를 채운다.

## 퀴즈 연결 방식

Main은 이미 `FMainEducationContent`에 퀴즈 데이터(질문·초성·정답)를 갖고 있다.
**데이터를 두 곳에 두지 않기 위해** Core 퀴즈 에셋을 새로 만들지 않고,
Main Content에서 `FInitialConsonantQuizDefinition`을 만들어 Core 런타임에 넘긴다.

```text
Scenario Quiz Interaction
        ↓
AMainEducationScenarioManagerActor::RequestVoiceRecognition(QuizID)   ← 지금까지 빈 구현
        ↓  FMainEducationContent → FInitialConsonantQuizDefinition
UInitialConsonantQuizComponent::StartQuizDefinition()   (Core, 마이크 + 패널 소유)
        ↓  OnQuizFinished
정답  → SubmitQuizAnswer(정답)      → CompleteInteraction → 다음 Step(Travel)
시도 소진 → 정답 공개 후 CompleteInteraction → 다음 Step(Travel)
취소  → 아무 것도 하지 않는다 (레벨 종료·재시작 경로)
```

의존 방향은 `Main(Core 계층) → Core/Quiz → Core/Voice`로 기존과 동일하다.
Game Feature를 참조하지 않는다.

# 변경 예정 파일

```text
Source/SuwonSiegeContestVR/Private/Main/Education/MainEducationScenarioDefinition.cpp
Source/SuwonSiegeContestVR/Public/Main/Education/MainEducationScenarioManagerActor.h
Source/SuwonSiegeContestVR/Private/Main/Education/MainEducationScenarioManagerActor.cpp
Source/SuwonSiegeContestVR/Private/Tests/MainEducationFlowTests.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/.../Public/Ongseong/OngseongDefenseScenarioManager.h
Plugins/GameFeatures/GF_OngseongCrossbow/.../Private/Tests/OngseongDefenseTests.cpp
Content/Data/DA_Scenario_MainEducation.uasset          (에디터에서 재생성)
Scripts/VerifyMainSingijeonOngseongOnly.py             (새 흐름 기준으로 갱신)
docs/Main/specs/MAIN_EDUCATION_FLOW.md
docs/Core/specs/INITIAL_CONSONANT_QUIZ.md
docs/Main/STATUS.md
```

# 구현 단계

1. `BuildDefaultContent()`를 새 흐름으로 교체하고 `QUIZ_HWACHA` · `QUIZ_ONGSEONG` Content와
   `Travel_Singijeon` · `Travel_Ongseong` Route를 추가한다.
2. `AMainEducationScenarioManagerActor`에 Core 퀴즈 컴포넌트를 붙이고 음성 포트를 구현한다.
   퀴즈 Step 동안 Main 프레젠테이션 패널을 숨긴다. `EndPlay`에서 퀴즈를 취소해 마이크를 놓는다.
3. 옹성 `bRunIntroQuiz` 기본값을 `false`로 바꾼다.
   `LV_Ongseong.umap`에 이 프로퍼티 오버라이드가 없음을 확인했으므로 레벨 재저장은 필요 없다.
4. 자동화 테스트를 새 흐름·퀴즈·이동 기준으로 갱신한다.
5. Editor 타깃 빌드 후 `Scripts/ConfigureMainSingijeonOngseongOnly.py`로 에셋을 재생성하고
   `Scripts/VerifyMainSingijeonOngseongOnly.py`로 검증한다.

# 다른 Feature에 미치는 영향

| 대상 | 영향 |
|---|---|
| `GF_Singijeon` | 없음. 기존 `DA_Experience_Singijeon`으로 진입·복귀한다 |
| `GF_OngseongCrossbow` | 레벨 자체 초성 퀴즈가 기본 꺼짐. 나레이션 → 돌격 흐름은 그대로 |
| `GF_Gongsimdon` / `GF_Geojunggi` | Main 설명 Stage에서 빠지지만 Level·Experience 에셋은 그대로 둔다 |
| Core Quiz / Voice | 변경 없음. 기존 인터페이스만 사용한다 |

# 검증 방법

| 항목 | 방법 |
|---|---|
| 흐름 구조 | `Suwon.Main.Education.DefaultFlow` — Stage/Step 순서, 퀴즈 위치, Travel 뒤 Step 존재, Route 연결 |
| 퀴즈 판정 | `Suwon.Core.Hangul.InitialConsonants` (기존) — "화차" → "ㅎ ㅊ" |
| 마이크 수명 | `Suwon.Core.Quiz.VoiceLifetime` (기존) |
| 옹성 회귀 | `SuwonSiegeContestVR.Ongseong.Defense.GatedAssaultStart` — 플래그를 켠 상태에서 기존 동작 유지 |
| 에셋 | `Scripts/VerifyMainSingijeonOngseongOnly.py` |
| 실기 | PIE에서 나레이션 종료 → 퀴즈 등장 → "화차" 발화 → `LV_Singijeon` 이동 → 복귀 → 옹성 퀴즈 |
