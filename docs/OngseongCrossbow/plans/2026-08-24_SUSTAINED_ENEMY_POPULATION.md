# 적 상시 유지 스폰 (Wave 폐지)

**작성일**: 2026-08-24 · **대상 Feature**: `GF_OngseongCrossbow`
**상태**: `In Progress` — C++ 구현 완료, 레벨 설정과 PIE 검증 대기. 하단 "진행 상태" 참조.
**대체 대상**: `docs/OngseongCrossbow/completed/2026-08-23_REPEATING_ENEMY_WAVES.md`의 반복 Wave 규칙

---

# 목적

적 생성을 "Wave 단위 전멸 후 재시작"에서 "옹성 내 적 인원을 상시 유지"로 바꾼다.

* 적은 최대 **15명**까지 동시에 존재한다.
* 적이 처치되면 스폰 지점에서 **몇 초 뒤 리스폰**하여 다시 전진한다.
* 방어 시간이 끝나거나 실패할 때까지 이 인원이 계속 유지된다.

---

# 현재 상태

| 항목 | 실제 구현 |
|---|---|
| `AOngseongEnemyWaveManager` | `SwordsmenToSpawn=10`, `ArchersToSpawn=10`, `TotalEnemiesToSpawn=20`을 `SpawnInterval=2.5`초 간격으로 스폰하고 `MaxActiveEnemies=6`으로 동시 수를 제한. 20명을 모두 스폰하면 `StopSpawning()` |
| 전멸 판정 | `AreAllEnemiesDefeated()`는 `DefeatedEnemyCount >= TotalEnemiesToSpawn`. 충족 시 `OnAllEnemiesDefeated` 방송 |
| `AOngseongDefenseScenarioManager` | `OnAllEnemiesDefeated`와 충차 파괴를 함께 추적해 `InterWaveDelay=3`초 뒤 `ResetWave()` + `StartSpawning()` + 충차 1대 재생성 (`TryScheduleNextWave`, `StartNextWave`, `CurrentWaveNumber`) |
| 충차 | `SpawnAndActivateRam()`이 `SpawnActor`로 생성하고 파괴 시 `Destroy()` (Wave 시작에 종속) |
| 사망 처리 | `HandleEnemyDeath()`가 즉시 Pool에 반환. 리스폰 개념 없음 |
| Pool 크기 | `Enemy_ActorPool` 8, 자동 확장 비활성. 궁병 Pool은 `Ongseong.ArcherPool` 태그로 탐색 |
| Automation | `OngseongEnemyWaveManagerTests`, `OngseongDefenseTests`가 Wave 기본값과 반복 규칙을 검증 |

동시 15명은 현재 Pool 상한(검병 8)을 초과하므로, **`2026-08-24_POOLED_INSTANCES_AND_FX` 계획의
Pool 크기 재산정이 선행되거나 동시에 진행되어야 한다.**

---

# 구현 범위

## 스폰 정책

| 설정 | 기본값 | 설명 |
|---|---|---|
| `MaxConcurrentEnemies` | 15 | 동시에 살아 있을 수 있는 적 총원 |
| `SwordsmanSlots` | 8 | 검병 유지 인원 |
| `ArcherSlots` | 7 | 궁병 유지 인원 |
| `InitialSpawnInterval` | 0.4초 | 시작 시 15명을 채울 때의 간격 (동시 스폰 스파이크 방지) |
| `RespawnDelay` | 5.0초 | 처치된 적이 다시 스폰되기까지의 대기 시간 |
| `RespawnDelayJitter` | 1.5초 | 리스폰이 한꺼번에 몰리지 않도록 하는 무작위 편차 |
| `bMaintainPopulation` | true | 상시 유지 동작 on/off (디버그용) |

* 적이 사망하면 즉시 Pool에 반환하고, **같은 유형**의 리스폰을 예약해 검병/궁병 비율을 유지한다.
* 리스폰된 적은 기존 스폰 지점에서 기존 대열 오프셋 규칙(`BuildSpawnTransform`)으로 등장해 다시 전진한다.
* 퇴각·실패·`ResetWave`·`EndPlay` 시 예약된 리스폰 타이머를 모두 취소한다.
* Pool이 고갈되어 스폰이 실패하면 해당 슬롯을 다음 리스폰 주기에 재시도한다.

## 충차 (2026-08-24 사용자 요구로 개정)

* 충차는 **1대뿐이며 리스폰하지 않는다.** 병사와 달리 재투입 대상이 아니다.
* 첫 접근은 **아주 느리다**(`MoveSpeed` 기본 35 cm/s). 총통으로 조준·파괴할 시간을 확보하기 위한 값이다.
* **충차 파괴가 이 체험의 클리어 조건이다.** 파괴 즉시 성공으로 전환하고 적을 퇴각시킨다.
* 180초 생존 성공 규칙은 폐지한다. 시간 제한은 선택 기능(`bUseDefenseTimeLimit`, 기본 off)으로만 남긴다.

## 방어 시나리오

* `[개정]` 성공은 충차 파괴, 실패는 성문 파괴다. 6초 자동 재시도와 성공 시 전원 퇴각 후 Experience 완료는 **그대로 유지**한다.
* Wave 관련 상태(`CurrentWaveNumber`, `bRepeatWavesDuringDefense`, `InterWaveDelay`,
  `TryScheduleNextWave`, `StartNextWave`, `NextWaveTimerHandle`, `bCurrentInfantryWaveDefeated`,
  `bCurrentRamDefeated`)는 제거한다.
* HUD/나레이션에서 "Wave n" 개념 대신 **누적 처치 수**와 **현재 적 수**를 사용한다.

## API 변경 요약

| 기존 | 변경 후 |
|---|---|
| `SwordsmenToSpawn` / `ArchersToSpawn` / `TotalEnemiesToSpawn` | `SwordsmanSlots` / `ArcherSlots` / `MaxConcurrentEnemies` |
| `MaxActiveEnemies=6` | `MaxConcurrentEnemies=15`로 통합 |
| `AreAllEnemiesDefeated()` | 제거 (상시 유지에서는 전멸 상태가 존재하지 않음) |
| `OnAllEnemiesDefeated` | 제거. 대신 `OnEnemyDefeated(TotalDefeated, CurrentAlive)` 추가 |
| `OnWaveProgress` | `OnPopulationChanged(CurrentAlive, MaxConcurrent)` |
| `OnWaveStarted` / `HasWaveStarted()` | `OnSpawningStarted` / `IsMaintainingPopulation()` |
| `GetCurrentWaveNumber()` (Manager) | `GetTotalDefeatedEnemies()` |

클래스 이름 `AOngseongEnemyWaveManager`는 **유지한다.** 레벨 배치 인스턴스와 Blueprint 참조가
걸려 있어 Rename은 불필요한 재저장·참조 갱신을 유발한다(`CLAUDE.md` 14절).
`RetreatAllEnemies` / `ReleaseAllEnemies` / `SetEnemyPool` / `SetObjectiveTarget` /
`SetArcherPrimaryTarget` 계약은 그대로 둔다.

---

# 변경 예정 파일

* `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/OngseongEnemyWaveManager.h`
* `.../Private/Ongseong/OngseongEnemyWaveManager.cpp`
* `.../Public/Ongseong/OngseongDefenseScenarioManager.h`
* `.../Private/Ongseong/OngseongDefenseScenarioManager.cpp`
* `.../Private/Tests/OngseongEnemyWaveManagerTests.cpp`
* `.../Private/Tests/OngseongDefenseTests.cpp`
* `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Maps/LV_Ongseong.umap` (Pool 크기·스포너 설정)
* `docs/OngseongCrossbow/ARCHITECTURE.md`, `STATUS.md`

---

# 구현 단계

1. `AOngseongEnemyWaveManager`의 카운터를 슬롯 기반 인구 유지 구조로 교체한다.
   (유형별 목표 인원, 현재 생존 목록, 유형별 리스폰 타이머 큐)
2. 시작 시 `InitialSpawnInterval` 간격으로 목표 인원까지 채우는 초기 스폰 루프를 만든다.
3. `HandleEnemyDeath`에서 Pool 반환 후 같은 유형의 리스폰을 `RespawnDelay ± Jitter`로 예약한다.
4. 퇴각/해제/`EndPlay` 경로에서 모든 리스폰 타이머를 정리한다.
5. `AOngseongDefenseScenarioManager`에서 Wave 재시작 코드를 제거하고, 충차 리스폰 타이머로 대체한다.
6. HUD/나레이션 연결을 누적 처치 수 기준으로 바꾼다.
7. Automation Test를 새 계약으로 갱신한다.
   (초기 인원 충원, 사망 후 리스폰 예약, 성공/실패 시 예약 취소, Pool 고갈 시 재시도)
8. 레벨의 Pool 크기와 스포너 설정을 새 기본값으로 조정한다.

---

# 다른 Feature에 미치는 영향

* 변경은 `GF_OngseongCrossbow`의 스포너와 시나리오 Manager 내부에 한정한다.
* Shared `AActorPool`, `AEnemyCombatCharacter`, `UHealthComponent`, `ACombatAIController` 계약은
  변경하지 않는다. (Pool **크기 설정**만 레벨 에셋에서 조정)
* 신기전 `ASingijeonEnemyWaveActor`와 공심돈 적 그룹은 별도 구현이므로 영향이 없다.

---

# 검증 방법

* `SuwonSiegeContestVREditor Win64 Development` 빌드
* `SuwonSiegeContestVR.Ongseong` Automation Test 갱신 후 전체 통과
* `LV_Ongseong_CombatTest`(비VR 테스트 GameMode)에서 PIE 확인
  * 시작 후 약 6초 내 적 15명 도달
  * 적 처치 후 `RespawnDelay` 뒤 같은 유형이 스폰 지점에서 재등장해 전진
  * 검병/궁병 비율 유지
  * 충차 파괴 시 성공 → 리스폰 중단 및 전원 퇴각, 성문 파괴 실패 시 리스폰 중단
  * `Enemy_ActorPool` 고갈 경고 없음
* `stat unit`으로 동시 15명 상태의 Game/Draw 시간 기록 (Android 실측은 별도 범위)

---

# 위험 요소

* 동시 15명 + 궁병 화살은 기존 예산(동시 6명)의 2.5배다. Quest 스탠드얼론에서 프레임 예산을
  초과할 수 있다. `2026-08-24_POOLED_INSTANCES_AND_FX`의 풀링과 함께 측정하고, 필요하면
  `MaxConcurrentEnemies`를 데이터로 낮춘다.
* NavMesh 위 15명 동시 이동은 경로 재계산 비용이 크다. 검병은 기존처럼 BT 없이 직접 이동 요청을
  유지해 비용을 억제한다.
* Pool 크기가 15 미만이면 스폰이 조용히 실패한다. Pool 고갈 로그(Plan 3)를 먼저 넣는 편이 안전하다.

---

# 진행 상태 (2026-08-24)

## 구현 완료 (C++)

`AOngseongEnemyWaveManager` (클래스 이름 유지, 내부 동작 교체)

* 슬롯 기반 인구 유지: `MaxConcurrentEnemies=15`, `SwordsmanSlots=8`, `ArcherSlots=7`
* 시작 시 `InitialSpawnInterval=0.4`초 간격으로 목표 인원까지 충원하고, 가득 차면 충원 타이머를 정지한다.
* 사망 시 Pool 반환 → **같은 유형**의 리스폰을 `RespawnDelay=5.0 ± RespawnDelayJitter=1.5`초로 예약
* Pool 고갈로 리스폰이 실패하면 인원을 줄이지 않고 다음 주기에 재시도한다.
* `StopSpawning` / `RetreatAllEnemies` / `ReleaseAllEnemies` / `ResetWave` / `EndPlay`에서 예약된 리스폰을 모두 취소한다.
* 유형 선택은 슬롯 대비 결손이 큰 쪽 우선(동률이면 검병)이다.

`AOngseongDefenseScenarioManager`

* Wave 반복 로직 제거: `TryScheduleNextWave`, `StartNextWave`, `CurrentWaveNumber`,
  `bRepeatWavesDuringDefense`, `InterWaveDelay`, `HandleWaveDefeated`, `NextWaveTimerHandle`
* 충차 재투입 추가: `bRespawnRamDuringDefense=true`, `RamRespawnDelay=20.0`, `IsRamRespawnPending()`,
  `RespawnRam()` (reflected 타이머 콜백)
* `[개정]` 성공 판정은 충차 파괴로 바뀌었다. 성문 파괴 실패, 6초 자동 재시도, 성공 시 퇴각 → Experience 완료는 그대로다.
* `GetTotalDefeatedEnemies()`로 누적 처치 수를 노출한다.

`UOngseongNarrationComponent`

* `OnSpawningStarted` / `OnEnemySpawned` / `OnPopulationChanged`로 재연결
* HUD 진행 표시를 "적 저지 (처치/전체)"에서 **"옹성 내 적 (현재/최대)"** 으로 변경
* 기존 `HandleAllEnemiesDefeated`의 성공 알림은 제거했다. 성공 나레이션과 HUD는 이미 시나리오 매니저가 담당한다.

## 실제 API 변경 결과

| 기존 | 구현 결과 |
|---|---|
| `SwordsmenToSpawn` / `ArchersToSpawn` / `TotalEnemiesToSpawn` | `SwordsmanSlots` / `ArcherSlots` / `MaxConcurrentEnemies` |
| `MaxActiveEnemies` | `MaxConcurrentEnemies`로 통합 |
| `AreAllEnemiesDefeated()` / `OnAllEnemiesDefeated` | 제거. `OnEnemyDefeated(TotalDefeated, LivingEnemies)` 추가 |
| `OnWaveProgress` | `OnPopulationChanged(LivingEnemies, MaxConcurrentEnemies)` |
| `OnWaveStarted` / `HasWaveStarted()` | `OnSpawningStarted` / `IsSpawningActive()` |
| `GetDefeatedEnemyCount()` | `GetTotalDefeatedEnemies()` |
| (신규) | `GetLivingEnemyCount()`, `GetLivingEnemyCountOfType()`, `GetPendingRespawnCount()`, `IsMaintainingPopulation()` |

`RetreatAllEnemies` / `ReleaseAllEnemies` / `ResetWave` / `SetEnemyPool` / `SetObjectiveTarget` /
`SetArcherPrimaryTarget` / `SpawnEnemy` / `IsSpawnConfigured`는 계약을 유지했다.

## Automation Test 갱신

* `SuwonSiegeContestVR.Ongseong.WaveManager.Configuration`: 15명·8/7 슬롯·5초 리스폰 기본값,
  Pool 고갈 시 인원 미증가, `ResetWave`의 예약 취소
* `SuwonSiegeContestVR.Ongseong.Defense.Contracts`: 충차 파괴 → 재투입 예약 → `RespawnRam` 실행,
  성문 파괴 시 예약 취소·스폰 중단·인구 정리

## 남은 작업

* 레벨의 Pool 크기 조정(검병 9 / 궁병 8) — `Scripts/CreateOngseongCombatTestMode.py`가 테스트 레벨에서 처리하며,
  본편 `LV_Ongseong`은 별도 확인이 필요하다.
* PIE에서 15명 유지·리스폰·성공 시 정지 확인
* Automation Test 실행 (에디터 타깃 빌드 필요)

## 검증 상태

* Game 타깃 빌드: **성공**
* Editor 타깃 빌드/Automation 실행: **보류** (Live Coding 활성)

---

# 진행 상태 갱신 (2026-08-24, 2차)

사용자 요구로 종료 규칙이 바뀌어 아래와 같이 다시 구현했다.

| 항목 | 결과 |
|---|---|
| 클리어 조건 | 충차 파괴 → `SucceedDefense()`. 180초 생존 성공 규칙 폐지 |
| 시간 제한 | `bUseDefenseTimeLimit`(기본 false). 켜면 `DefenseDuration` 경과 시 실패 |
| 충차 리스폰 | **제거**. `RamRespawnDelay`/`RespawnRam`/`IsRamRespawnPending` 삭제, `IsRamDestroyed()` 추가 |
| 충차 접근 속도 | `MoveSpeed` 120 → **35 cm/s** |
| 실패 처리 | `FailDefense(NarrationEvent, Headline, Detail, Notification)`로 통합. 성문 파괴와 (선택적) 시간 초과가 공유 |
| 적 병사 | 변경 없음. 동시 15명 유지 + 5초 ± 1.5초 리스폰 |

## 검증 결과

* `SuwonSiegeContestVREditor Win64 Development` 빌드: 성공
* Automation **24/24 통과** (`SuwonSiegeContestVR` 전체, exit code 0)
* 비VR 테스트 레벨 헤드리스 구동 로그
  * `Defense started. Ram=BP_OngseongRam_C_2, enemy slots=15, time limit=off`
  * `Enemy spawning started: 8 swordsmen + 7 archers, respawn 5.0s (+/-1.5)`
  * `Ongseong population filled: 15 living enemies`
  * `Ram advancing toward BP_OngseongGate_C_0 at 35 cm/s`
  * `LogActorPool` 고갈 경고 0건

## 테스트 하네스에서 확인한 사항

`AActor::ProcessEvent`는 `World->AreActorsInitialized()`가 false인 동안 UFUNCTION 호출(다이내믹 델리게이트 포함)을 조용히 버린다.
자동화 테스트 월드에서 `InitializeActorsForPlay` + `BeginPlay`를 호출하지 않으면 사망/목표 콜백이 전달되지 않는다.
`OngseongDefenseTests`에 이를 추가했다. 기존 반복 Wave 테스트가 통과한 적이 없던 이유도 동일하다.
