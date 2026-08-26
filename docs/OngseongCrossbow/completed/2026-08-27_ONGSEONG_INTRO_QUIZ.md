# 완료 기록 — 옹성 체험 시작 초성 퀴즈

**완료일**: 2026-08-27
**Feature**: `GF_OngseongCrossbow` (Core 초성 퀴즈 런타임 사용)
**관련 문서**: `docs/Core/completed/2026-08-27_INITIAL_CONSONANT_QUIZ.md`,
`docs/Core/specs/INITIAL_CONSONANT_QUIZ.md`

---

# 작업

옹성 본편 진행에 **초성 퀴즈 단계**를 추가했다.

```text
총통 포탄 장전 완료 (ReadyToAim)
        ↓
교관 체험 시작 나레이션 (TrainingCompleted)
        ↓
초성 퀴즈 "ㅇ ㅅ"  ← 이번에 추가
        ↓
AssaultStartDelay (기본 2초)
        ↓
나팔 · 전투 BGM · 적 웨이브 · 충차 전진
```

# 구현 내용

`AOngseongDefenseScenarioManager`

| 항목 | 내용 |
|---|---|
| `IntroQuiz` | Core `UInitialConsonantQuizComponent` 서브오브젝트 |
| 기본 퀴즈 | `QUIZ_ONGSEONG` — 질문 "성문 바깥을 둘러싸 지키는 이 방어시설의 이름은?", 정답 "옹성" |
| 초성 표시 | 정답에서 자동 추출되어 `ㅇ ㅅ`로 표시된다. 별도 입력이 필요 없다 |
| `bRunIntroQuiz` | 기본 `true`. 끄면 이전 흐름과 완전히 동일하다 |
| `IntroQuizID` | 실행할 퀴즈 항목. 다른 퀴즈로 교체 가능 |
| `StartIntroQuiz()` | Blueprint에서도 호출 가능. 퀴즈가 시작되면 `true` |
| `IsIntroQuizComplete()` | 재시도 시 재출제를 막는다 |

동작 규칙

* 퀴즈는 **교관 나레이션이 끝난 뒤** 시작하므로 질문 음성과 마이크가 겹치지 않는다.
* 퀴즈가 끝나기 전에는 나팔·BGM·적 스폰이 시작되지 않는다.
  전투 소음 속에서 음성 인식을 하지 않기 위한 배치다.
* 오답으로 시도(기본 3회)를 모두 쓰면 정답을 공개하고 그대로 진행한다. 체험이 막히지 않는다.
* 방어 실패 후 자동 재시도할 때는 다시 묻지 않는다.
* 퀴즈·마이크는 `EndPlay`에서 확실히 종료된다.
* 퀴즈를 시작할 수 없는 경우(항목 없음 등)에는 경고 로그를 남기고 곧바로 돌격으로 넘어간다.

# 변경 파일

```text
Plugins/GameFeatures/GF_OngseongCrossbow/.../Public/Ongseong/OngseongDefenseScenarioManager.h
Plugins/GameFeatures/GF_OngseongCrossbow/.../Private/Ongseong/OngseongDefenseScenarioManager.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/.../Private/Tests/OngseongDefenseTests.cpp
```

`OngseongDefenseScenarioManager.cpp`에는 `#include "Engine/GameInstance.h"`도 추가했다.
이 파일이 unity 빌드에서 제외될 때 드러나는 누락 include였다.

# 주요 결정 사항

* 퀴즈 **런타임은 Core**, **질문 데이터만 Feature**가 소유한다. 의존 방향은 `GF → Core` 그대로다.
* 퀴즈 위치를 "나레이션 종료 ~ 돌격 사이"로 정한 이유는 음성 인식이 전투 소음·나레이션과
  겹치지 않는 유일한 구간이기 때문이다.
* 기본값을 켜진 상태(`bRunIntroQuiz = true`)로 두되 한 플래그로 되돌릴 수 있게 했다.

# 테스트 결과

**Editor 타깃 빌드 성공. 프로젝트 Automation 32건 전부 통과 (Exit Code 0).**

`SuwonSiegeContestVR.Ongseong.Defense.GatedAssaultStart`에 다음 단계를 추가했고 통과했다.

* 장전 완료 후에도 돌격이 시작되지 않는다
* 퀴즈가 열리고 초성이 `ㅇ ㅅ`로 표시된다 (정답에서 자동 추출)
* 퀴즈가 떠 있는 동안 `DefenseState`는 `Idle`이다
* `"옹성"` 제출이 정답으로 인정되고 `IsIntroQuizComplete()`가 참이 된다
* 퀴즈 종료와 함께 마이크가 해제된다
* 재호출 시 다시 출제되지 않는다(재시도 대비)

옹성 기존 테스트 5건(Chongtong.LoadingSequence, Defense.Contracts, Interaction.Prompts,
RangedCombat.Contracts, WaveManager.Configuration)도 회귀 없이 통과했다.

**테스트 환경 주의**: 빈 테스트 월드에는 나레이션 플레이어가 없어 `IsNarrationBusy()`가
계속 참이므로, 나레이션 종료 → 퀴즈 자동 시작 구간은 테스트로 재현되지 않는다.
테스트는 `StartIntroQuiz()`를 직접 호출해 이후 흐름을 검증한다.
실제 레벨에서는 `OnNarrationIdle`(또는 `BriefingTimeout`)이 같은 경로를 호출한다.

# 남은 문제

* **실기 확인 필요**: 본편 `LV_Ongseong`에서 교관 나레이션(`ON_08`) 종료 후 퀴즈가 뜨는지,
  패널 거리·크기가 총통 조작 시점의 시야에서 적절한지 HMD로 확인해야 한다.
* 실제 음성 인식 백엔드는 아직 Mock이다. PC 테스트는 콘솔 `ssv.voice.submit 옹성`으로 한다.
  Mock 기본값은 `ManualOnly`라 아무 입력이 없으면 8초 × 3회 후 정답을 공개하고 진행한다.
* 퀴즈 패널의 최종 아트(WBP)와 정답/오답 사운드는 미제작이다.
