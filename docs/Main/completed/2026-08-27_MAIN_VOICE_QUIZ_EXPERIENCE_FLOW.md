# 완료 기록 — Main 음성 초성 퀴즈 + 체험 이동 (화차 · 옹성)

**완료일**: 2026-08-27
**계층**: Main (Core 계층) — Core 퀴즈·음성 런타임 사용
**관련 문서**: `docs/Main/plans/2026-08-27_MAIN_VOICE_QUIZ_EXPERIENCE_FLOW.md`,
`docs/Core/specs/INITIAL_CONSONANT_QUIZ.md`, `docs/Main/specs/MAIN_EDUCATION_FLOW.md`

---

# 작업

Main 교육 흐름에 음성 인식 초성 퀴즈를 연결하고, 정답을 맞히면 해당 체험 Level로 이동하게 했다.

```text
성문 앞 인사 (MAIN_NA_01)
        ↓
신기전 설명 화면 + 나레이션 (MAIN_NA_13 ~ 17, "…화차에 장착하면 한꺼번에 발사")
        ↓
초성 퀴즈 "ㅎ ㅊ"  → 정답 "화차"
        ↓
LV_Singijeon 체험 → 시나리오 종료 시 L_Main 복귀
        ↓
신기전 체험 완료 화면
        ↓
옹성 설명 화면 + 나레이션 (MAIN_NA_18 ~ 24, "…직접 체험해 보겠습니다")
        ↓
초성 퀴즈 "ㅇ ㅅ"  → 정답 "옹성"
        ↓
LV_Ongseong 체험 → 방어 성공 시 L_Main 복귀
        ↓
옹성 체험 완료 화면 → 교육 마무리 (MAIN_NA_25 ~ 30)
```

퀴즈는 **설명 나레이션이 끝난 뒤**에만 열린다. 질문 음성과 마이크가 겹치지 않는다.

# 구현 내용

## Main 흐름 데이터 (`UMainEducationScenarioDefinition::BuildDefaultContent`)

Stage 6개 → 4개. 사용자 결정에 따라 거중기·녹로·공심돈 설명 Stage를 제거하고
신기전·옹성 두 체험 흐름만 남겼다. 성문 앞 인사와 교육 마무리는 흐름의 시작·끝이라 유지했다.

| Stage | Step |
|---|---|
| `MAIN_GATE` | `GATE_GREETING` (Narration `MAIN_NA_01`) |
| `SINGIJEON` | `SINGIJEON_IMAGE` → `SINGIJEON_NARRATION`(`MAIN_NA_13`) → **`SINGIJEON_QUIZ`** → **`TRAVEL_SINGIJEON`** → `AFTER_SINGIJEON` |
| `ONGSEONG` | `ONGSEONG_IMAGE` → `ONGSEONG_NARRATION`(`MAIN_NA_18`) → **`ONGSEONG_QUIZ`** → **`TRAVEL_ONGSEONG`** → `AFTER_ONGSEONG` |
| `SUMMARY` | `SUMMARY_01` → `SUMMARY_NARRATION`(`MAIN_NA_25`) |

새 퀴즈 Content 2건.

| ID | 질문 | 초성 | 정답 |
|---|---|---|---|
| `QUIZ_HWACHA` | 여러 발의 신기전을 한꺼번에 발사하기 위해 만든 이동식 발사대의 이름은? | `ㅎ ㅊ` | 화차 |
| `QUIZ_ONGSEONG` | 성문 바깥을 한 겹 더 둘러싸 지키는 이 방어시설의 이름은? | `ㅇ ㅅ` | 옹성 |

`ExperienceRoutes`에 `Travel_Singijeon` → `DA_Experience_Singijeon`,
`Travel_Ongseong` → `DA_Experience_Ongseong`을 연결했다. 둘 다 `ReturnLevel = L_Main`이다.

**Travel Step 뒤에는 반드시 Step이 하나 더 있어야 한다.** `BeginExperienceTravel()`이
`NextInteractionID`를 복귀 체크포인트로 저장하므로, Travel이 Stage의 마지막이면 이동 자체가
거부된다. `AFTER_SINGIJEON` / `AFTER_ONGSEONG`이 그 자리이자 복귀 지점이다.

## 음성 퀴즈 연결 (`AMainEducationScenarioManagerActor`)

지금까지 비어 있던 음성 포트를 Core 퀴즈 런타임에 연결했다.

| 항목 | 내용 |
|---|---|
| `EducationQuiz` | Core `UInitialConsonantQuizComponent` 서브오브젝트. 패널·시도·마이크를 소유 |
| `RequestVoiceRecognition()` | 현재 Step의 `FMainEducationContent`를 `FInitialConsonantQuizDefinition`으로 변환해 `StartQuizDefinition()` 호출 |
| `CancelVoiceRecognition()` | `CancelQuiz()` — 어떤 경로로 끝나든 마이크를 놓는다 |
| `HandleEducationQuizFinished()` | 정답 → `SubmitQuizAnswer()` / 시도 소진 → 정답 공개 후 `CompleteQuizInteraction()` / 취소 → 아무 것도 하지 않음 |
| `CompleteQuizInteraction()` | 퀴즈 Step을 닫고 Scenario를 진행시키는 단일 지점 |
| `EndPlay()` | 퀴즈 취소. 퀴즈와 마이크가 레벨보다 오래 살아남지 않는다 |
| `bUseVoiceQuiz` / `QuizListenDuration` / `QuizMaxAttempts` | 디자이너 조정용. 기본 켜짐 / 8초 / 3회 |

**퀴즈 데이터는 한 곳에만 둔다.** Core 퀴즈 에셋을 새로 만들지 않고 Main Content
(질문·초성·정답)에서 런타임 퀴즈를 만든다. 디자이너는 지금까지처럼 `01 Editor Flow`만 편집하면 된다.

퀴즈 Step 동안에는 Main 프레젠테이션 패널을 숨기고 Core 퀴즈 패널만 띄운다.

## 옹성 레벨 중복 퀴즈 해제

`AOngseongDefenseScenarioManager::bRunIntroQuiz` 기본값을 `false`로 바꿨다.
Main이 옹성 퀴즈를 내므로 레벨 안에서 같은 질문을 반복하지 않는다.
`LV_Ongseong.umap`에 이 프로퍼티의 인스턴스 오버라이드가 없음을 확인했으므로 레벨은 재저장하지 않았다.
단독 실행이나 회귀 테스트를 위해 `SetRunIntroQuiz(true)`를 추가했다.

## 부수 수정

`RebuildRuntimeFromEditorFlow()`가 Step마다 Content를 추가하면서 같은 ContentID가 중복 등록되고 있었다
(이미지 Step과 그 나레이션 Step이 같은 화면을 공유하기 때문). 그 결과
`ValidateEducationScenario()`가 항상 "duplicate ID"로 실패해 검증이 무의미했다.
런타임 조회는 원래 첫 항목만 쓰므로 **첫 항목만 등록**하도록 고쳤다. 동작 변화는 없고 검증이 다시 유효해졌다.

# 변경 파일

```text
Source/SuwonSiegeContestVR/Public/Main/Education/MainEducationScenarioDefinition.h
Source/SuwonSiegeContestVR/Private/Main/Education/MainEducationScenarioDefinition.cpp
Source/SuwonSiegeContestVR/Public/Main/Education/MainEducationScenarioManagerActor.h
Source/SuwonSiegeContestVR/Private/Main/Education/MainEducationScenarioManagerActor.cpp
Source/SuwonSiegeContestVR/Private/Tests/MainEducationFlowTests.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/.../Public/Ongseong/OngseongDefenseScenarioManager.h
Plugins/GameFeatures/GF_OngseongCrossbow/.../Private/Tests/OngseongDefenseTests.cpp
Content/Data/DA_Scenario_MainEducation.uasset
Scripts/VerifyMainSingijeonOngseongOnly.py
docs/Main/plans/2026-08-27_MAIN_VOICE_QUIZ_EXPERIENCE_FLOW.md
docs/Main/specs/MAIN_EDUCATION_FLOW.md
docs/Core/specs/INITIAL_CONSONANT_QUIZ.md
docs/Main/STATUS.md
docs/OngseongCrossbow/STATUS.md
```

에셋 재생성은 기존 `Scripts/ConfigureMainSingijeonOngseongOnly.py`가 그대로 수행한다(수정 없음).

# 주요 결정 사항

| 결정 | 이유 |
|---|---|
| 퀴즈 데이터를 **Main Content에 그대로 두고** 런타임만 Core에 넘긴다 | 질문·정답이 두 곳으로 갈라지지 않는다. 디자이너 편집 지점도 하나로 유지된다 |
| 퀴즈 UI는 **Core 패널**, Main 패널은 숨김 | 이미 있는 패널을 재사용한다. 두 패널이 겹쳐 보이지 않는다 |
| 오답 3회 → **정답 공개 후 그대로 이동** | 교육 콘텐츠에서 오답이 진행을 막으면 안 된다. 마이크가 없어도 흐름이 멈추지 않는다 |
| 옹성 레벨 자체 퀴즈는 **기본 끔** | 같은 질문을 두 번 묻지 않는다. 플래그 하나로 되돌릴 수 있다 |
| Travel Step 뒤 `AFTER_*` Step 유지 | 복귀 체크포인트가 없으면 이동 자체가 거부된다 |
| 거중기·녹로·공심돈 설명 Stage 제거 | 사용자 결정. 연결된 체험이 없는 설명 구간이었다 |

# 테스트 결과

**Editor 타깃 빌드 성공. Automation 34건 전부 통과 (EXIT CODE: 0).**

| 테스트 | 결과 | 내용 |
|---|---|---|
| `Suwon.Main.Education.DefaultFlow` | Success | Stage 순서, "설명 → 퀴즈 → 이동 → 복귀 Step" 배치, 나레이션 시작 Row, 초성 표시와 정답 판정("화차"/"옹성" 인정, "성벽" 거부), Route 2건 연결, 제거된 Content 부재 |
| `SuwonSiegeContestVR.Ongseong.Defense.GatedAssaultStart` | Success | 플래그를 켠 상태에서 기존 퀴즈 → 돌격 흐름 회귀 없음 |
| `Suwon.Core.Hangul.InitialConsonants`, `Suwon.Core.Quiz.VoiceLifetime` | Success | Core 회귀 없음 |
| 기존 30건 | Success | 회귀 없음 |

실행 명령
```text
UnrealEditor-Cmd.exe SuwonSiegeContestVR.uproject
  -ExecCmds="Automation RunTests Suwon+SuwonSiegeContestVR;Quit"
  -unattended -nopause -nosplash -NullRHI -NoSound
```

**에셋 검증**: `ConfigureMainSingijeonOngseongOnly.py` 실행 후
`VerifyMainSingijeonOngseongOnly.py` 전 항목 통과 (`VERIFY_MAIN_SINGIJEON_ONGSEONG_ONLY SUCCESS`).
Stage/Step 순서, 퀴즈 초성·정답, Route의 `ReturnLevel = L_Main`, `L_Main`의 Manager 설정,
`LV_Ongseong`의 `bRunIntroQuiz = false`까지 확인했다.

**복귀 경로 확인**: `LV_Singijeon`의 `BP_ScenarioManager`에 `ScenarioExperienceBridgeComponent`가
`DA_Experience_Singijeon` · `CompleteExperienceOnScenarioFinished = true`로 연결되어 있고,
옹성은 `AOngseongDefenseScenarioManager::FinishSuccessfulRetreat()`가
`CompleteCurrentExperience(true)`를 호출한다. 두 체험 모두 L_Main으로 돌아온다.

# 남은 문제

1. **실기(HMD) 확인 필요.** 나레이션 종료 → 퀴즈 등장 타이밍, 퀴즈 패널 거리·크기,
   "화차" 발화 인식률, 이동 직후 VR 전환 체감을 PIE와 실기에서 확인해야 한다.
   마이크 없이 확인하려면 콘솔 `ssv.voice.submit 화차`, 상태는 `ssv.voice.status`.
2. **나레이션 음원과 화면 내용의 불일치.** 흐름에서 빠진 거중기·녹로·공심돈을
   `MAIN_NA_01`(건설 장치 예고)과 `MAIN_NA_26`(오늘 배운 것 정리)이 여전히 언급한다.
   음원 담당자가 해당 Row를 다시 녹음하거나 구간을 조정해야 완전히 맞는다.
   화면 텍스트는 실제 체험한 내용(신기전·화차·옹성)에 맞춰 이미 수정했다.
3. 퀴즈 패널의 최종 아트(WBP)와 정답/오답 사운드는 여전히 미제작이다.
4. 진행도는 세션 메모리라 앱을 재시작하면 처음부터 시작한다.
5. ~~옹성 체험을 **실패**하면 자동 재시도로 레벨에 남는다. Main으로 돌아오는 경로는 성공뿐이다.~~
   → **해결 (2026-08-27)**: 옹성 전투에 30초 상한을 두어 승패와 무관하게 Main으로 복귀한다.
   같은 작업에서 체험 복귀 시 성문 인트로 연출도 건너뛴다.
   `docs/OngseongCrossbow/completed/2026-08-27_ONGSEONG_BATTLE_TIME_LIMIT.md` 참조.
