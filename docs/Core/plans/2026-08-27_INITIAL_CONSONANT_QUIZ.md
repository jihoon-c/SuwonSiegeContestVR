# 초성 퀴즈 + 음성 인식(Mock) 구현 계획

**작성일**: 2026-08-27
**Feature**: Core (+ 첫 사용처: `GF_OngseongCrossbow`)
**관련 문서**: `docs/Core/specs/INITIAL_CONSONANT_QUIZ.md`, `docs/Core/specs/VOICE_RECOGNITION_BACKEND_SURVEY.md`

---

**상태**: 2026-08-27 구현 완료 — `docs/Core/completed/2026-08-27_INITIAL_CONSONANT_QUIZ.md` 참조

---

# 목적

음성 인식 기반 초성 퀴즈를 Core에 구현하고, 옹성 시나리오의
**총통 장전 완료 → 교관 체험 시작 나레이션 → 초성 퀴즈("ㅇㅅ") → 돌격 개시** 흐름에 연결한다.
음성 인식은 목 구현으로 두되, 향후 sherpa-onnx 계열 백엔드로 교체할 수 있게 인터페이스를 고정한다.

# 현재 상태

* `EScenarioInteractionType::Quiz` / `VoiceCommand` 열거값은 존재하지만 **런타임이 없다.**
* `UMainEducationScenarioDefinition`에 초성·정답 데이터와 `IsAcceptedQuizAnswer()`가 있으나
  Main 전용이고 위젯·마이크 제어가 없다. `RequestVoiceRecognition()`은 의도적으로 빈 구현이다.
* 옹성은 `AOngseongDefenseScenarioManager::HandleTrainingLoadingStateChanged()` →
  `ScheduleAssaultAfterBriefing()` → `HandleBriefingNarrationIdle()` → `AssaultStartDelay` →
  `BeginAssault()` 순서로 게이트되어 있다. 퀴즈를 넣을 자리는 나레이션 종료와 돌격 사이다.

# 구현 범위

포함:
1. Core 한글 유틸리티(초성 추출·정답 정규화)
2. Core 음성 인식 베이스 + Mock 구현
3. Core 초성 퀴즈 데이터/컴포넌트/네이티브 위젯
4. Scenario `Quiz` 인터랙션 브리지
5. 옹성 시나리오 연결(교관 나레이션 뒤 "ㅇㅅ" 퀴즈)
6. 자동화 테스트

제외(별도 범위):
* 실제 sherpa-onnx / Vosk 임포트, 마이크 권한 흐름
* Main 교육 흐름의 퀴즈를 Core 런타임으로 이관
* 최종 UI 아트(WBP) 제작 — 네이티브 위젯이 기본값으로 동작

# 변경 예정 파일

신규 (Core):
```
Source/SuwonSiegeContestVR/Public|Private/Core/Text/HangulTextLibrary.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Voice/VoiceRecognitionTypes.h
Source/SuwonSiegeContestVR/Public|Private/Core/Voice/VoiceRecognitionComponent.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Voice/MockVoiceRecognitionComponent.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Quiz/InitialConsonantQuizTypes.h
Source/SuwonSiegeContestVR/Public|Private/Core/Quiz/InitialConsonantQuizSet.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Quiz/InitialConsonantQuizComponent.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Quiz/InitialConsonantQuizWidget.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Quiz/ScenarioQuizBridgeComponent.h|.cpp
Source/SuwonSiegeContestVR/Private/Tests/InitialConsonantQuizTests.cpp
```

수정:
```
Plugins/GameFeatures/GF_OngseongCrossbow/.../OngseongDefenseScenarioManager.h|.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/.../Tests/OngseongDefenseTests.cpp
```

# 구현 단계

1. `UHangulTextLibrary` — 초성 추출(19자), 정답 정규화, 정답 일치 판정
2. `FVoiceRecognitionRequest/Result` + `UVoiceRecognitionComponent`
   * `StartListening/StopListening/IsListening`
   * `BeginBackendListening`/`EndBackendListening` `BlueprintNativeEvent`
   * 모든 백엔드 공통 결과 진입점 `ReportRecognizedText()`
   * 타임아웃 타이머, 키워드 매칭, `FindVoiceRecognitionComponent()` 정적 탐색
3. `UMockVoiceRecognitionComponent` — 지연 후 자동 정답/오답, `ssv.voice.submit` 콘솔 명령
4. 퀴즈 데이터/에셋/컴포넌트/위젯
   * 위젯은 시야 정면 200cm World Space 배치, 회전만 추종
   * 마이크는 퀴즈 진행 중에만 On
5. `UScenarioQuizBridgeComponent` — `Quiz` 인터랙션 ↔ 퀴즈 결과 보고
6. 옹성 연결
   * `AOngseongDefenseScenarioManager`에 `UInitialConsonantQuizComponent` 서브오브젝트 추가
   * 기본 퀴즈 `QUIZ_ONGSEONG` (초성 자동 추출 = "ㅇ ㅅ") 내장
   * `HandleBriefingNarrationIdle()`에서 퀴즈 시작, `OnQuizFinished`에서 기존 돌격 지연 경로로 합류
   * `bRunIntroQuiz`로 끌 수 있게 해 기존 동작을 보존
7. 자동화 테스트 및 문서 갱신

# 다른 Feature에 미치는 영향

* Core에 **추가만** 한다. 기존 Core 클래스의 시그니처를 바꾸지 않는다.
* 옹성 매니저에는 기본값 `bRunIntroQuiz = true`인 단계가 하나 늘어난다.
  퀴즈를 끄면 기존 흐름과 동일하다.
* Main 교육 흐름은 이번 변경으로 동작이 달라지지 않는다.
* 다른 Feature는 컴포넌트 2개를 추가하는 것만으로 자기 시퀀스에 퀴즈를 넣을 수 있다.

# 검증 방법

* Editor/Game 타깃 빌드
* `Suwon.Core.Hangul.InitialConsonants` — "옹성" → "ㅇ ㅅ", 쌍자음, 비한글 혼용
* `Suwon.Core.Quiz.VoiceLifetime` — 퀴즈 시작 시 마이크 On, 종료 시 Off
* `SuwonSiegeContestVR.Ongseong.Defense.GatedAssaultStart` — 나레이션 종료 후 퀴즈가 돌격을 막고,
  정답 시 돌격이 진행되는지
* PIE: 콘솔 `ssv.voice.submit 옹성`으로 정답 경로 수동 확인
