**상태: 완료 (2026-08-26)** — 결과는
[completed/2026-08-26_MAIN_LEVEL_PARITY_AND_GATED_ASSAULT.md](../completed/2026-08-26_MAIN_LEVEL_PARITY_AND_GATED_ASSAULT.md)
참조. 실제 구현에서 달라진 점: `SC_WarHorn` Sound Cue는 Python으로 만들 수 없어 나팔 기본값을
`WarHorn` Sound Wave로 두었고, `low_poly_mountain_7`은 지형 서브레벨 소유라 별도 추가가 불필요했다.

# 목적

1. 본편 `LV_Ongseong`의 레벨 배치를 검증된 `LV_Ongseong_CombatTest`와 동일하게 맞춘다.
2. 본편 흐름을 "교관 지시 → 총통 장전 → 체험 시작 나레이션 → 2초 뒤 나팔·BGM·적 웨이브 → 충차 파괴 시 미션 클리어·체험 종료"로 바꾼다.

# 현재 상태

## 배치 (2026-08-26 헤드리스 덤프 기준)

`LV_Ongseong` 124개 / `LV_Ongseong_CombatTest` 123개(지형 서브레벨 제외) Actor.

| 항목 | LV_Ongseong | LV_Ongseong_CombatTest |
|---|---|---|
| 남한산성 지형 | 없음 | `/Game/Maps/Main/L_NamhansanseongLandscape` Streaming |
| `BP_OngseongSpawnPoint` | 0개 | 3개 (EnemyInitial / SoldierRespawn / Ram) |
| `PlayerStart` | 0개 | 2개 (`PlayerStart`, `DebugCamera_PlayerStart`) |
| 성벽·성문·총통·아군 Transform | 구 좌표 | 지형 위로 이동 (대략 dx -542, dy -8789, dz +1050) |
| 잔여 `BP_EnemyArcher`/`BP_EnemySword` | 각 1개 배치 | 없음 |
| `ground`/`background`/구 배틀먼트 2개 | 있음 | 없음 |
| `low_poly_mountain_7` | 없음 | 있음 |
| WaveManager Spawn Point 연결 | 없음 | 3개 연결 |
| GameMode Override | 없음 (전역 `BP_XRGameMode`) | `BP_OngseongCombatTestGameMode` (비VR 디버그) |

## 진행 흐름

- `AOngseongDefenseScenarioManager::BeginPlay`가 `bAutoStart`로 즉시 `StartDefense()`를 호출한다.
  충차 스폰과 적 웨이브가 레벨 진입 즉시 시작된다.
- 나레이션 체인이 `ON_01→…→ON_09`(도입) 뒤 `WaveStarted→ON_10→ON_11→ON_12→ON_13`이라
  **장전 지도가 적 등장 이후**에 나온다.
- BGM `SC_BGM`은 두 레벨의 **Level Blueprint BeginPlay → PlaySound2D**에서 재생된다
  (CLAUDE.md 7절 위반이며 요구되는 타이밍과도 다르다).
- 나팔 사운드용 Sound Cue가 없다. `WarHorn`은 Sound Wave다.

# 구현 범위

- 배치 동기화는 **Transform·추가·삭제·지형 서브레벨·Spawn Point 연결**까지로 한다.
  본편의 VR GameMode(Override 없음)는 유지하고 디버그 전용 `DebugCamera_PlayerStart`는 옮기지 않는다.
- 시작 게이트, 나팔·BGM 재생, 성공 종료는 `AOngseongDefenseScenarioManager`에 넣는다.
  Level Blueprint에는 Gameplay Logic을 두지 않는다.
- 나팔·BGM은 Blueprint Class Defaults에서 Sound Cue로 교체할 수 있게 노출한다.

# 변경 예정 파일

```text
Plugins/.../Public/Ongseong/OngseongDefenseScenarioManager.h
Plugins/.../Private/Ongseong/OngseongDefenseScenarioManager.cpp
Plugins/.../Public/Ongseong/OngseongNarrationComponent.h
Plugins/.../Private/Ongseong/OngseongNarrationComponent.cpp
Plugins/.../Private/Tests/OngseongDefenseTests.cpp
Plugins/.../Content/Asset/Sound/SC_WarHorn.uasset            (신규)
Plugins/.../Content/Blueprints/BP_OngseongDefenseScenarioManager.uasset
Plugins/.../Content/Data/DT_OngseongNarration.uasset
Plugins/.../Content/Maps/LV_Ongseong.umap
Plugins/.../Content/Maps/LV_Ongseong_CombatTest.umap          (Level BP BGM 노드 제거만)
Scripts/CreateOngseongAssaultAudio.py                         (신규)
Scripts/SyncOngseongMainLevelLayout.py                        (신규)
Scripts/VerifyOngseongMainLevelFlow.py                        (신규)
docs/OngseongCrossbow/completed/2026-08-26_MAIN_LEVEL_PARITY_AND_GATED_ASSAULT.md
```

# 구현 단계

1. **나레이션 체인 재구성** (`DT_OngseongNarration`)
   - 도입: `ON_01→02→03→04→05→06→07→09→12→13`(정지)
   - 장전 이벤트: `PowderLoaded→ON_14`, `RammingCompleted→ON_15`, `ReadyToAim→ON_16→ON_17`
   - 신규 `TrainingCompleted→ON_08`("지금부터 실제 상황을 가정한 수비 훈련을 시작하겠습니다") = 체험 시작 나레이션
   - 적 등장: `WaveStarted→ON_10→ON_11`(정지)
   - 클리어: `DefenseSucceeded→ON_22`
2. **`UOngseongNarrationComponent`**
   - `OnNarrationIdle` 델리게이트와 `IsNarrationBusy()` 추가 (큐가 비고 재생이 끝난 시점 통지)
   - `TrainingCompleted → ON_08` 기본 바인딩 추가
3. **`AOngseongDefenseScenarioManager`**
   - `bStartAfterChongtongLoaded`(기본 false), `TrainingCannon`, `AssaultStartDelay`(기본 2.0),
     `TrainingNarrationTimeout` 추가
   - `AssaultHornSound`, `BattleMusic`, 볼륨·페이드 프로퍼티를 `EditAnywhere/BlueprintReadWrite`로 추가
   - `StartDefense()` 진입 시 나팔·BGM 재생, 종료 시 BGM 페이드 아웃
   - 장전 완료(`ReadyToAim`) → `TrainingCompleted` 나레이션 → 나레이션 종료 후 `AssaultStartDelay` → `StartDefense()`
   - `SuccessCompletionDelay`로 클리어 나레이션이 끝난 뒤 체험을 종료
4. **오디오 에셋**: `SC_WarHorn` Sound Cue 생성, `BP_OngseongDefenseScenarioManager` 기본값에 `SC_WarHorn`·`SC_BGM` 지정
5. **Level Blueprint**: 두 레벨의 `BeginPlay → PlaySound2D(SC_BGM)` 제거
6. **`LV_Ongseong` 배치 동기화**: 지형 서브레벨 연결, Transform 이관, 추가·삭제, Spawn Point 3종 배치·연결,
   본편 인스턴스에 `bStartAfterChongtongLoaded=true`·WaveManager `bAutoStart=false` 설정
7. 검증 스크립트 + 자동화 테스트

# 다른 Feature에 미치는 영향

- `DT_OngseongNarration`, `BP_OngseongDefenseScenarioManager`는 옹성 Feature 전용이며
  `LV_Ongseong_CombatTest`도 함께 영향을 받는다. CombatTest는 `bStartAfterChongtongLoaded=false`로
  종전처럼 즉시 전투를 시작한다.
- Core/Shared Gameplay 클래스는 수정하지 않는다.
- `/Game/Maps/Main/L_NamhansanseongLandscape`는 읽기 전용으로 참조만 추가한다.

# 검증 방법

- UBT 컴파일, `SuwonSiegeContestVR.Ongseong` 자동화 테스트
- `Scripts/VerifyOngseongMainLevelFlow.py`로 배치 파리티·프로퍼티·사운드 할당 확인
- 헤드리스 `LV_Ongseong` 로드 후 Map Check
