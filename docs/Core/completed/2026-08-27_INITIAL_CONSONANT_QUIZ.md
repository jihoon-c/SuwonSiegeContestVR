# 완료 기록 — 초성 퀴즈 + 음성 인식(Mock)

**완료일**: 2026-08-27
**계층**: Core (첫 사용처 `GF_OngseongCrossbow`)
**관련 문서**: `docs/Core/plans/2026-08-27_INITIAL_CONSONANT_QUIZ.md`,
`docs/Core/specs/INITIAL_CONSONANT_QUIZ.md`, `docs/Core/specs/VOICE_RECOGNITION_BACKEND_SURVEY.md`

---

# 작업

음성 인식으로 답하는 초성 퀴즈를 Core 런타임으로 구현하고, 옹성 시나리오의
총통 장전 → 교관 나레이션 → 초성 퀴즈 → 돌격 흐름에 연결했다.
음성 인식 백엔드는 Mock이며, 교체 지점을 인터페이스로 고정했다.

# 구현 내용

## Core / Text

`UHangulTextLibrary` — 한글 초성 추출과 정답 정규화.

* `ExtractInitialConsonants("옹성")` → `"ㅇ ㅅ"` (U+AC00 음절 분해, 표준 초성 19자)
* `NormalizeAnswer()` — 공백·문장부호 제거 후 소문자화
* `DoesAnswerMatch()` — 완전 일치만 인정(부분 일치 거부)

## Core / Voice

* `FVoiceRecognitionRequest` / `FVoiceRecognitionResult` — 백엔드 무관 요청·결과
* `UVoiceRecognitionComponent` (Abstract) — 요청 관리, 청취 타임아웃, 키워드 매칭, 상태 전이
  * 백엔드 훅: `BeginBackendListening` / `EndBackendListening` (BlueprintNativeEvent)
  * 공통 결과 진입점: `ReportRecognizedText()`
  * 탐색기: `FindVoiceRecognition()` — Pawn → PlayerController → 레벨 Actor 순
* `UMockVoiceRecognitionComponent` — 현재 백엔드
  * `ManualOnly`(기본) / `AutoCorrect` / `AutoNoMatch`
  * 기본값에서는 아무 것도 인식하지 않으므로 퀴즈가 자체 타임아웃으로 진행된다(8초 × 3회 후 정답 공개)
  * 콘솔 명령 `ssv.voice.submit <텍스트>` (Shipping 제외)

## Core / Quiz

* `FInitialConsonantQuizDefinition` — 퀴즈 1건. **정답만 넣으면 초성은 자동 추출**
* `UInitialConsonantQuizSet` — 공용 퀴즈 라이브러리 Data Asset(중복 ID·정답 누락 검증 포함)
* `UInitialConsonantQuizComponent` — 구동부
  * `StartQuiz(QuizID)` / `StartQuizDefinition()` / `SubmitAnswer()` / `GiveUp()` / `CancelQuiz()`
  * 이벤트 `OnQuizStarted` / `OnQuizAttempt` / `OnQuizFinished`
  * 마이크는 퀴즈 시작 시에만 켜지고, 정답·시도 소진·포기·취소·`EndPlay` 어느 경로로 끝나든 꺼진다
  * 레벨에 인식기가 없으면 Mock을 런타임 생성해 흐름이 멈추지 않게 한다
  * `bShowQuizPanel = false`로 자체 UI만 쓰는 체험도 지원
* `UInitialConsonantQuizWidget` — 네이티브 World Space 패널
  * "초성 퀴즈" 라벨 / 질문 / **초성(폰트 150)** / 상태 문구 / 힌트·남은 기회·정답 공개
  * 시야 정면 200cm, 눈높이 -15cm에 **한 번 배치**하고 회전만 30Hz로 추종(헤드 락 없음)
* `UScenarioQuizBridgeComponent` — `EScenarioInteractionType::Quiz` 인터랙션 ↔ 퀴즈 결과 자동 중계

## GF_OngseongCrossbow

`AOngseongDefenseScenarioManager`

* `IntroQuiz` 서브오브젝트(Core 컴포넌트)와 기본 퀴즈 `QUIZ_ONGSEONG`(정답 "옹성", 초성 자동 "ㅇ ㅅ")
* 흐름: 총통 장전 완료 → `TrainingCompleted` 나레이션 → **초성 퀴즈** → `AssaultStartDelay` → 나팔·BGM·적 웨이브
* `bRunIntroQuiz`를 끄면 기존 흐름과 완전히 동일
* 실패 재시도 시에는 다시 묻지 않는다(`bIntroQuizComplete` 유지)
* `EndPlay`에서 퀴즈를 취소해 마이크가 체험보다 오래 살아남지 않게 한다

# 변경 파일

신규
```text
Source/SuwonSiegeContestVR/Public|Private/Core/Text/HangulTextLibrary.h|.cpp
Source/SuwonSiegeContestVR/Public/Core/Voice/VoiceRecognitionTypes.h
Source/SuwonSiegeContestVR/Public|Private/Core/Voice/VoiceRecognitionComponent.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Voice/MockVoiceRecognitionComponent.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Quiz/InitialConsonantQuizTypes.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Quiz/InitialConsonantQuizSet.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Quiz/InitialConsonantQuizComponent.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Quiz/InitialConsonantQuizWidget.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Quiz/ScenarioQuizBridgeComponent.h|.cpp
Source/SuwonSiegeContestVR/Private/Tests/InitialConsonantQuizTests.cpp
docs/Core/specs/INITIAL_CONSONANT_QUIZ.md
docs/Core/specs/VOICE_RECOGNITION_BACKEND_SURVEY.md
docs/Core/plans/2026-08-27_INITIAL_CONSONANT_QUIZ.md
```

수정
```text
Plugins/GameFeatures/GF_OngseongCrossbow/.../Public/Ongseong/OngseongDefenseScenarioManager.h
Plugins/GameFeatures/GF_OngseongCrossbow/.../Private/Ongseong/OngseongDefenseScenarioManager.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/.../Private/Tests/OngseongDefenseTests.cpp
Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h    (빌드 복구 — 아래 참조)
Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp (빌드 복구 — 아래 참조)
docs/ARCHITECTURE.md (3.6절, 구현 현황표, 리스크 5번)
docs/COLLABORATION.md (미해결 5번)
docs/OngseongCrossbow/STATUS.md
```

**빌드 복구 (다른 작업자의 미커밋 변경)**

`VRPlayerPawn.h`에 새로 추가돼 있던 `GetPhoneAnchorLocation()`의 인라인 본문이
전방 선언만 있는 `UMotionControllerComponent`를 역참조해 **프로젝트 전체 빌드가 실패**하고 있었다.
기능은 그대로 두고 **본문만 `.cpp`로 옮겼다**(무거운 헤더를 공용 헤더에 끌어들이지 않기 위해).
선언·`UFUNCTION` 지정자·동작은 변경하지 않았다.

# 주요 결정 사항

| 결정 | 이유 |
|---|---|
| 퀴즈·음성을 **Core**에 둔다 | 4개 체험 모두가 쓰는 콘텐츠 핵심 기능이고, Feature 의존이 없다 |
| 초성을 **정답에서 자동 추출** | 다른 개발자가 퀴즈를 추가할 때 입력해야 할 데이터가 정답 하나로 줄어든다 |
| 음성 인식을 **컴포넌트 상속으로 교체** | Mock → sherpa-onnx 전환 시 게임플레이 코드가 바뀌지 않는다 |
| 결과 진입점을 **`ReportRecognizedText()` 하나**로 통일 | Mock·콘솔·실제 백엔드가 같은 판정 경로를 쓴다 |
| 패널을 **월드 고정 + 회전만 추종** | VR에서 헤드 락 UI는 불편하고 멀미를 유발한다 |
| Main 교육 흐름은 **이번에 건드리지 않음** | 다른 담당 범위이며, 이관은 별도 판단이 필요하다 |
| 시도 소진 시에도 **진행은 계속** | 교육 콘텐츠라 오답이 체험을 막으면 안 된다. 정답을 공개하고 넘어간다 |

# 테스트 결과

**Editor 타깃 빌드 성공. 프로젝트 Automation 32건 전부 통과 (Exit Code 0).**

| 테스트 | 결과 | 내용 |
|---|---|---|
| `Suwon.Core.Hangul.InitialConsonants` | Success | 초성 추출(쌍자음·공백·비한글·구분자), 정규화, 정답 판정, 퀴즈 표시 초성 |
| `Suwon.Core.Quiz.VoiceLifetime` | Success | 퀴즈 시작 시 마이크 On, 오답 후 재청취, 정답·시도 소진·취소 시 마이크 Off |
| `SuwonSiegeContestVR.Ongseong.Defense.GatedAssaultStart` (확장) | Success | 퀴즈 시작 → "ㅇ ㅅ" 표시 → 정답 → 완료 플래그 → 재출제 안 함 → 돌격 진행 |
| 기존 29건 (Core / Gameplay / Main / Gongsimdon / Singijeon / Ongseong) | Success | 회귀 없음 |

실행 명령
```text
UnrealEditor-Cmd.exe SuwonSiegeContestVR.uproject
  -ExecCmds="Automation RunTests Suwon+SuwonSiegeContestVR;Quit"
  -unattended -nopause -nosplash -NullRHI -NoSound
```

**테스트 환경 주의**: 빈 테스트 월드에는 나레이션 플레이어가 없어
`UOngseongNarrationComponent::IsNarrationBusy()`가 계속 참이다. 따라서 교관 나레이션 →
퀴즈 자동 전환은 테스트에서 재현되지 않으며, 테스트는 `StartIntroQuiz()`를 직접 호출해
그 이후 흐름을 검증한다. 실제 레벨에서는 나레이션 종료 시 `OnNarrationIdle`이,
최악의 경우 `BriefingTimeout`(기본 30초)이 같은 경로를 호출한다.

# 남은 문제

1. **Game(비에디터) 타깃은 이번 작업 이전부터 깨져 있다.**
   `Core/Scenario/ScenarioInteractableComponent.cpp:23`이 `WITH_EDITORONLY_DATA` 전용
   `UPrimitiveComponent::IsVisualizationComponent()`를 호출한다.
   **Android 패키징 시 문제가 되므로 별도 조치가 필요하다.** 이번 범위가 아니라 수정하지 않았다.
2. 실제 온디바이스 음성 인식 백엔드 미임포트 (Mock 사용 중).
   PC 테스트는 콘솔 `ssv.voice.submit 옹성`을 사용한다.
3. 마이크 권한(Android `RECORD_AUDIO`) 요청 흐름 미설계
4. 최종 UI 아트(WBP)와 정답/오답 사운드 미제작 — 네이티브 패널이 기본값으로 동작
5. Main 교육 흐름(`UMainEducationScenarioDefinition`)의 퀴즈를 Core 런타임으로 이관할지는 미결정
