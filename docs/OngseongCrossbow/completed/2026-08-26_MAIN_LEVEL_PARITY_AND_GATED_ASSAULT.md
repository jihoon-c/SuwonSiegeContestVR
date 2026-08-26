# 작업

1. 본편 `LV_Ongseong`의 레벨 배치를 검증된 `LV_Ongseong_CombatTest`와 동일하게 맞췄다.
2. 본편 진행을 "교관 지시 → 총통 장전 → 체험 시작 나레이션 → 2초 뒤 나팔·BGM·적 웨이브 →
   충차 파괴 시 미션 클리어 나레이션 → 체험 종료"로 바꿨다.

# 구현 내용

## 1. 레벨 배치 동기화

`Scripts/SyncOngseongMainLevelLayout.py`가 테스트 레벨을 원본으로 삼아 본편에 적용한다.
하드코딩된 좌표가 아니라 실행 시점의 `LV_Ongseong_CombatTest`를 스냅샷해서 반영하므로
테스트 레벨이 다시 바뀌면 재실행만 하면 된다.

| 항목 | 결과 |
|---|---|
| 공용 남한산성 지형 | `/Game/Maps/Main/L_NamhansanseongLandscape`를 `LevelStreamingAlwaysLoaded`로 연결 |
| 공통 Actor Transform | 121개 중 95개 이동 (성벽·성문·총통·아군·조명·NavMesh 볼륨 등) |
| 삭제 | `ground`, `background`, `low_poly_mountain_3`, `Battlement_GateDefense29/56`, 잔여 `BP_OngseongArcher`, `BP_EnemySword` |
| 추가 | `BP_OngseongSpawnPoint` ×3 (EnemyInitial / SoldierRespawn / Ram), `PlayerStart` |
| 연결 | WaveManager `InitialSpawnPoint`·`SoldierRespawnPoint`, Scenario `RamSpawnPoint` |

`low_poly_mountain_7`은 본편에 추가하지 않았다. 그 Actor는 테스트 레벨 자체가 아니라
연결된 지형 서브레벨 소유이며, 지형을 붙이는 순간 본편에도 함께 들어온다.

**의도적으로 가져오지 않은 것** (사용자 확인 완료):

- GameMode Override `BP_OngseongCombatTestGameMode`. 비VR 디버그 자유카메라 모드라
  본편에 적용하면 VR 체험이 동작하지 않는다. 본편은 전역 `BP_XRGameMode`를 그대로 쓴다.
- `DebugCamera_PlayerStart`. 위 디버그 GameMode 전용이며, 본편에 두면 VR 플레이어가
  엉뚱한 지점에서 시작할 수 있다.

`GroupActor`와 `RecastNavMesh`의 Transform은 옮기지 않는다. 전자는 멤버 Actor로부터
에디터가 다시 계산하는 캐시값이고, 후자는 생성 데이터다. 멤버 Actor는 모두 정렬되어 있다.

## 2. 시작 게이트 (`AOngseongDefenseScenarioManager`)

새 프로퍼티:

| 프로퍼티 | 기본값 | 설명 |
|---|---|---|
| `bStartAfterChongtongLoaded` | `false` | 켜면 `BeginPlay`가 전투를 시작하지 않고 장전 완료를 기다린다. `bAutoStart`는 무시된다. |
| `TrainingCannon` | 비움 | 비워 두면 `IsPlayerOperable()`인 총통을 자동 탐색한다. |
| `AssaultStartDelay` | `2.0` | 체험 시작 나레이션이 끝난 뒤 나팔까지의 간격. |
| `BriefingTimeout` | `30.0` | 나레이션이 재생되지 않는 환경에서도 체험이 멈추지 않게 하는 안전장치. |
| `BriefingNarrationEvent` | `TrainingCompleted` | 체험 시작 나레이션 이벤트 이름. |
| `SuccessCompletionDelay` | `10.0` | 미션 클리어 나레이션이 끝날 시간을 준 뒤 Experience를 종료한다. `0`이면 종전처럼 적 퇴각 시점에 종료한다. |

동작 순서:

```text
BeginPlay  → 도입 나레이션(ON_01…) · 적 없음 · 충차 없음 · 총통 장전 지도
장전 완료(ReadyToAim) → TrainingCompleted → ON_08 "지금부터 실제 상황을 가정한 수비 훈련을 시작하겠습니다"
나레이션 종료 + 2초 → StartDefense()
  ├ 나팔 사운드 1회 재생
  ├ 전투 BGM 페이드 인
  ├ 충차 스폰 및 돌격
  └ 적 웨이브 스폰 → WaveStarted → ON_10 "적이 쳐들어오고있습니다!" → ON_11
충차 파괴 → DefenseSucceeded → ON_22 · BGM 페이드 아웃 · 10초 뒤 Experience 종료
```

`UOngseongNarrationComponent`에 `OnNarrationIdle` 델리게이트와 `IsNarrationBusy()`를 추가해
"나레이션이 끝난 뒤 2초"를 정확히 잴 수 있게 했다.

`AChongtongCannonActor::IsPlayerOperable()`을 추가했다. 자동 사격이 꺼진 총통이 플레이어용이며,
아군 총통과 구분하는 유일한 런타임 근거다.

## 3. 나팔·BGM 저작 위치

`BP_OngseongDefenseScenarioManager` → **Class Defaults → `Ongseong|Scenario|Audio`**:

| 슬롯 | 현재 기본값 |
|---|---|
| `Assault Horn Sound` | `WarHorn` |
| `Battle Music` | `SC_BGM` |
| `Assault Horn Volume` / `Battle Music Volume` | `1.0` |
| `Battle Music Fade In Time` / `Fade Out Time` | `1.5` / `3.0` |

두 슬롯 모두 `USoundBase`라 **Sound Cue를 그대로 지정할 수 있다.** 위 기본값은
플레이스홀더이며, `EditAnywhere`이므로 Class Defaults뿐 아니라 `LV_Ongseong`에 배치된
`Ongseong_DefenseScenario` 인스턴스의 Details 패널에서도 레벨별로 덮어쓸 수 있다.

BGM은 종전에 두 레벨의 **Level Blueprint `BeginPlay → PlaySound2D(SC_BGM)`** 에서 재생됐다.
이는 CLAUDE.md 7절 위반이자 요구된 타이밍과도 달라 두 레벨 모두에서 노드를 제거했다.

## 4. 나레이션 순서 재구성 (`DT_OngseongNarration`)

자막·녹음 음성은 그대로 두고 `NextRow`/`AdvanceMode`만 바꿨다.

| 행 | 이전 NextRow | 현재 NextRow |
|---|---|---|
| `ON_07` | `ON_08` | `ON_09` |
| `ON_08` | `ON_09` | `None` (체험 시작 나레이션으로 분리) |
| `ON_09` | `None` | `ON_12` |
| `ON_11` | `ON_12` | `None` |

결과 흐름:

```text
도입   ON_01→02→03→04→05→06→07→09→12→13
장전   PowderLoaded ON_14 · RammingCompleted ON_15 · ReadyToAim ON_16→ON_17
체험시작 TrainingCompleted ON_08
적 등장 WaveStarted ON_10→ON_11
클리어  DefenseSucceeded ON_22
```

# 변경 파일

```text
Plugins/.../Public/Ongseong/OngseongDefenseScenarioManager.h
Plugins/.../Private/Ongseong/OngseongDefenseScenarioManager.cpp
Plugins/.../Public/Ongseong/OngseongNarrationComponent.h
Plugins/.../Private/Ongseong/OngseongNarrationComponent.cpp
Plugins/.../Public/Ongseong/ChongtongCannonActor.h
Plugins/.../Private/Ongseong/OngseongEnemyWaveManager.cpp      (기존 컴파일 오류 수정)
Plugins/.../Private/Tests/OngseongDefenseTests.cpp             (GatedAssaultStart 추가)
Plugins/.../Content/Blueprints/BP_OngseongDefenseScenarioManager.uasset
Plugins/.../Content/Data/DT_OngseongNarration.uasset
Plugins/.../Content/Maps/LV_Ongseong.umap
Plugins/.../Content/Maps/LV_Ongseong_CombatTest.umap           (Level BP BGM 노드 제거)
Scripts/ConfigureOngseongAssaultStart.py                        (신규)
Scripts/SyncOngseongMainLevelLayout.py                          (신규)
Scripts/VerifyOngseongMainLevelFlow.py                          (신규)
Scripts/InspectOngseongLevelParity.py                           (신규 · 조사용)
Scripts/InspectOngseongFlowSetup.py                             (신규 · 조사용)
docs/OngseongCrossbow/plans/2026-08-26_MAIN_LEVEL_PARITY_AND_GATED_ASSAULT.md
```

레벨 백업: `Saved/CodexBackups/2026-08-26_OngseongMainLevelParity/LV_Ongseong.umap`

# 주요 결정 사항

- **배치 동기화는 맵 복제가 아니라 Actor 단위 반영으로 했다.** 본편 패키지와 Actor GUID,
  월드 세팅, VR GameMode를 유지한 채 배치만 맞추기 위해서다.
- **`bStartAfterChongtongLoaded` 기본값은 `false`다.** `LV_Ongseong_CombatTest`는 종전처럼
  즉시 전투를 시작해야 전투 검증이 가능하다. 본편 인스턴스에서만 켰다.
- **나팔·BGM은 `StartDefense()` 안에서 재생한다.** 실패 후 자동 재시도에서도 다시 울리고,
  게이트를 쓰지 않는 테스트 레벨에서도 같은 슬롯이 그대로 동작한다.
- **성공 종료를 타이머로 바꿨다.** 종전에는 적이 모두 퇴각한 시점에 Experience를 종료해서
  클리어 나레이션이 잘릴 수 있었다. `SuccessCompletionDelay = 0`으로 두면 이전 동작이다.
- **Sound Cue 에셋은 생성하지 않고 슬롯만 노출했다.** 큐는 사운드 담당자가 이미 보유하고 있고,
  Python으로는 온전한 Sound Cue를 만들 수 없다 (`USoundCue::AllNodes`가 protected라 스크립트로
  만든 큐는 그래프가 빈 채로 열리고 다음 에디터 저장에서 노드를 잃는다).
- **공용 지형 `L_NamhansanseongLandscape`는 참조만 추가했고 저장하지 않았다.**

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공.
- `SuwonSiegeContestVR` 자동화 테스트 **29/29 성공, 실패 0**.
  신규 `SuwonSiegeContestVR.Ongseong.Defense.GatedAssaultStart` 포함.
- `Scripts/VerifyOngseongMainLevelFlow.py` **19개 검사 전부 PASS**:
  - 본편 전용 잔여 Actor 0개 / 테스트 레벨 Actor 전부 존재
  - 공유 Actor 121개 Transform 일치 (파생 Transform 클래스 제외)
  - 지형 서브레벨 연결됨, 본편 GameMode Override 없음
  - Spawn Point 3역할 배치·연결, WaveManager auto-start off, 게이트 on, 지연 2.0초
  - 나팔·BGM 슬롯 지정됨
  - 나레이션 체인 재구성 확인, **23개 행 모두 녹음 음성 유지**
  - 두 레벨 Level Blueprint에 `PlaySound2D` 없음
- 헤드리스 `LV_Ongseong` 로드 시 이 작업으로 생긴 경고 없음.

## 이 작업 중 발견해 고친 기존 오류

`ecfb515` 시점의 `OngseongEnemyWaveManager.cpp:682`가 `TMap::RemoveAll`을 호출해
**브랜치가 컴파일되지 않는 상태였다.** UE 5.8 `TMap`에는 해당 멤버가 없다.
`CreateIterator` + `RemoveCurrent`로 교체했다. 동작은 동일하다.

# 남은 문제

- **나팔·BGM Sound Cue 할당은 사운드 담당자 작업으로 남긴다.** 현재 값은 레벨이 무음으로
  돌지 않게 넣어둔 플레이스홀더(`WarHorn` Sound Wave, `SC_BGM`)다. 두 슬롯 모두 `USoundBase`라
  준비된 Sound Cue를 그대로 지정하면 되고, `Scripts/ConfigureOngseongAssaultStart.py`를
  다시 실행하지 않는 한 스크립트가 덮어쓰지 않는다.
- **Navigation 재빌드가 필요할 수 있다.** `NavMeshBoundsVolume`의 위치와 스케일이 바뀌었으므로
  에디터에서 `LV_Ongseong`을 열고 Build > Build Paths를 한 번 실행한 뒤 저장하기를 권한다.
- **실제 HMD 검증이 남았다.** 나팔·BGM 볼륨 밸런스, `AssaultStartDelay` 2초의 체감,
  `SuccessCompletionDelay` 10초가 ON_22 길이에 맞는지는 실기 확인이 필요하다.
- 기존 경고: `BP_EnemySoldier`가 삭제된 `EnemyAILODComponent`·`EnemyBehaviorStateComponent`·
  `EnemySimpleMovementComponent`를 참조한다. 이 작업 범위 밖이며 그대로 두었다.
