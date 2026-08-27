# 완료 기록 — 옹성 전투 시간 상한과 메인 복귀

**완료일**: 2026-08-27
**Feature**: `GF_OngseongCrossbow` (Main 교육 흐름 연계)
**관련 문서**: `docs/Main/completed/2026-08-27_MAIN_VOICE_QUIZ_EXPERIENCE_FLOW.md`,
`docs/Main/specs/MAIN_EDUCATION_FLOW.md`

---

# 작업

옹성 체험의 전투를 **30초로 제한**하고, 시간이 되면 결과와 무관하게 메인 레벨로 돌아가
마지막 인사말(교육 마무리)로 이어지게 했다.

```text
옹성 레벨 진입
        ↓
총통 장전 훈련 → 교관 나레이션        ← 시간 제한 없음
        ↓
나팔 · 전투 BGM · 적 웨이브 시작       ← 여기서 30초 카운트 시작
        ↓ (30초)
전투 정리 → 즉시 L_Main 복귀
        ↓
"옹성 체험 완료" 화면 → 교육 마무리 나레이션(MAIN_NA_25~30)
```

# 구현 내용

## `AOngseongDefenseScenarioManager`

| 항목 | 내용 |
|---|---|
| `BattleTimeLimit` | 전투 상한(초). 기본 **30**. 0이면 상한 없이 기존처럼 충차 파괴로 끝난다 |
| 카운트 시작 | `StartDefense()` — 나팔이 울리고 적이 몰려오는 시점. 훈련·나레이션 구간은 포함하지 않는다 |
| `FinishExperienceNow()` | 전투를 정리하고 메인으로 넘기는 공개 진입점. Blueprint에서도 호출할 수 있다 |
| `RequestReturnToMain()` | `CompleteCurrentExperience(true)`를 부르는 **단일 복귀 지점**. 성공 복귀와 시간 상한 복귀가 함께 쓴다 |
| `GetRemainingBattleTime()` | 남은 상한 시간. UI나 디버그용 |

시간이 되면 다음을 수행한다.

* 상한·방어·자동재시도·성공대기 타이머를 모두 정리한다
* 충차를 멈추고(`StopRam`) 적을 전부 반환한다(`ReleaseAllEnemies` → 스폰도 함께 멈춘다)
* 전투 BGM을 끈다
* VR HUD에 "체험 시간이 끝났습니다"를 띄운다
* `CompleteCurrentExperience(true)`로 `DA_Experience_Ongseong.ReturnLevel`(=`L_Main`)로 이동한다

**즉시 복귀**다. 성공 연출("방어 성공" 나레이션 + `SuccessCompletionDelay` 10초)은 타지 않는다.
30초 안에 충차를 파괴하면 기존 성공 경로가 먼저 동작하고, 상한은 `bCompletionRequested` 가드로
한 번만 넘긴다. 방어에 실패해도 30초가 되면 자동 재시도 대신 메인으로 돌아온다.

## `AMainLevelIntroActor`

`bSkipIntroOnExperienceReturn`(기본 켜짐)을 추가했다.
Main Scenario 복귀 체크포인트가 남아 있으면 성문 인트로 카메라 연출을 건너뛰고 곧바로
`StartEducationAfterIntro()`로 넘어간다.

체험에서 돌아올 때마다 성곽 조감 → 타이틀 → 페이드 연출이 다시 재생되고 입력이 잠기던 문제를 없앤다.
신기전 복귀에도 같이 적용된다. 처음 `L_Main`에 들어올 때는 체크포인트가 없으므로 인트로가 정상 재생된다.

# 변경 파일

```text
Plugins/GameFeatures/GF_OngseongCrossbow/.../Public/Ongseong/OngseongDefenseScenarioManager.h
Plugins/GameFeatures/GF_OngseongCrossbow/.../Private/Ongseong/OngseongDefenseScenarioManager.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/.../Private/Tests/OngseongDefenseTests.cpp
Source/SuwonSiegeContestVR/Public/Main/Education/MainLevelIntroActor.h
Source/SuwonSiegeContestVR/Private/Main/Education/MainLevelIntroActor.cpp
docs/OngseongCrossbow/STATUS.md
docs/Main/specs/MAIN_EDUCATION_FLOW.md
```

# 주요 결정 사항

| 결정 | 이유 |
|---|---|
| 카운트를 **나팔(전투 시작)부터** 센다 | 훈련과 교관 나레이션까지 잘리면 체험 내용이 전달되지 않는다 |
| 시간이 되면 **즉시 복귀** | 시간이 정확해지고, 승패와 무관하게 같은 길이로 끝난다 |
| 기존 `bUseDefenseTimeLimit`를 **쓰지 않고 새 상한을 추가** | 그 값은 만료를 **방어 실패 + 자동 재시도**로 처리한다. 목적이 반대다 |
| 복귀 호출을 `RequestReturnToMain()` **한 곳으로** 모음 | 성공·시간초과 두 경로가 같은 가드를 공유해 이중 이동을 막는다 |
| 복귀 시 **인트로 연출 생략** | 체험을 마치고 돌아온 플레이어에게 시작 연출을 다시 보여주면 흐름이 끊긴다 |

# 테스트 결과

**미실행 — 에디터가 Live Coding으로 모듈을 점유하고 있어 빌드가 막혀 있다.**
UHT는 통과했다(선언부 검증 완료). 에디터를 닫거나 `Ctrl+Alt+F11`로 Live Coding 컴파일을 돌린 뒤
아래 항목을 실행하고 이 절을 갱신해야 한다.

`SuwonSiegeContestVR.Ongseong.Defense.GatedAssaultStart`에 다음을 추가했다.

* 돌격이 시작되면 전투 상한 타이머가 돈다 (`GetRemainingBattleTime() > 0`)
* 상한 처리 후 적 스폰이 멈추고 상한 타이머가 정리된다
* 복귀는 한 번만 요청된다 (두 번째 `FinishExperienceNow()`는 `false`)

기존 옹성 테스트 5건과 프로젝트 전체 Automation 회귀도 함께 확인해야 한다.

빈 테스트 월드에는 `UExperienceSubsystem`이 없어 레벨 이동 자체는 재현되지 않는다.
테스트는 타이머가 부르는 것과 같은 진입점을 직접 호출해 그 결과를 검사한다.

# 남은 문제

* **실기 확인 필요**: 30초가 체험 분량으로 적절한지, 전투 도중 끊기는 느낌이 어떤지 HMD로 확인해야 한다.
  길이를 바꾸려면 `BP_OngseongDefenseScenarioManager`의 `Ongseong|Scenario > Battle Time Limit`만 조정한다.
* 30초 상한 때문에 충차 파괴 성공 연출("방어 성공" 나레이션)은 대부분 보이지 않는다.
  성공을 보여주고 싶다면 상한을 늘리거나 `SuccessCompletionDelay`를 줄인다.
* 시간 종료 전용 나레이션은 없다. HUD 문구만 뜬다.
