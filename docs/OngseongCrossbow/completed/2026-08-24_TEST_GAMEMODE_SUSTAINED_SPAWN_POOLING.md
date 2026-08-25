# 작업

옹성 체험에 세 가지를 반영했다.

1. VR HMD·VR Pawn 없이 전투를 관찰할 수 있는 비VR 테스트 GameMode와 테스트 레벨
2. Wave 폐지 — 옹성 내 적을 상시 유지하고 처치 시 리스폰
3. 생성/소멸 인스턴스와 파티클의 오브젝트 풀링

작업 중 사용자 요구로 **종료 규칙을 "적 충차 파괴 = 클리어"로 변경**하고, 충차는 리스폰하지 않으며
첫 접근을 아주 느리게 하도록 함께 반영했다.

---

# 구현 내용

## 1. 비VR 전투 테스트 GameMode (Core + Feature)

* `ADebugFreeCameraPawn` — 충돌 없는 자유 비행 관전 카메라. WASD/방향키, E·Space 상승, Q 하강,
  마우스 시점, Shift ×4 / Ctrl ×0.25. 입력은 축 매핑이나 IMC 없이 `APlayerController` 폴링으로 처리해
  프로젝트 입력 설정에 의존하지 않는다.
* `ADebugFreeCameraGameMode` — `DefaultPawnClass`를 위 Pawn으로, `HUDClass`는 없음.
  HMD 비활성화는 XRBase 플러그인 헤더 대신 `GEngine->StereoRenderingDevice`를 사용해
  Core가 플러그인 모듈에 의존하지 않게 했다.
* Feature: `BP_OngseongCombatTestGameMode`(위 클래스의 자식)와 `LV_Ongseong_CombatTest`.
  본편 `LV_Ongseong`과 `Config/DefaultEngine.ini`의 전역 GameMode는 건드리지 않았다.
* 플레이어는 어떤 게임플레이 Actor에도 빙의하지 않는다.

## 2. 적 상시 유지 스폰 (Wave 폐지)

`AOngseongEnemyWaveManager` (클래스 이름은 레벨 참조 보존을 위해 유지)

* 동시 15명(검병 8 · 궁병 7)을 상시 유지. `MaxConcurrentEnemies` / `SwordsmanSlots` / `ArcherSlots`
* 시작 시 `InitialSpawnInterval` 0.4초 간격으로 충원, 가득 차면 충원 타이머 정지
* 사망 시 Pool 반환 후 **같은 유형**을 `RespawnDelay` 5.0초 ± `RespawnDelayJitter` 1.5초로 리스폰 예약
* Pool 고갈로 리스폰이 실패하면 인원을 줄이지 않고 다음 주기에 재시도
* 퇴각·해제·`ResetWave`·`EndPlay`에서 예약된 리스폰 전부 취소
* 델리게이트: `OnSpawningStarted` / `OnEnemySpawned` / `OnEnemyDefeated` / `OnPopulationChanged` / `OnAllEnemiesRetreated`

## 3. 종료 규칙 개정 (사용자 요구)

`AOngseongDefenseScenarioManager`

* **성공 = 적 충차 파괴.** `HandleRamDefeated` → `SucceedDefense()` → 전원 퇴각 → Experience 완료
* 충차는 1대이며 **리스폰하지 않는다.** 재투입 로직과 Wave 반복 로직을 모두 제거했다.
* 실패 = 성문 Health 0. `FailDefense(NarrationEvent, Headline, Detail, Notification)`로 통합했다.
* 시간 제한은 선택 기능(`bUseDefenseTimeLimit`, 기본 false). 켜면 `DefenseDuration` 경과 시 실패한다.
* 충차 접근 속도 `MoveSpeed` 120 → **35 cm/s**

## 4. 풀링과 FX

* Shared `UCombatFXLibrary` — Niagara 원샷 이펙트를 엔진 Component Pool(`ENCPoolMethod::AutoRelease`)로 재생.
  사운드는 선택적 `USoundConcurrency`를 받는다. 옹성의 폭발·발사 FX를 여기로 옮겼다.
* Shared `AActorPool` — 고갈 경고 로그(`LogActorPool`), `OnPoolExhausted`, `GetExhaustedRequestCount()`,
  `GetPooledActorClass()`, **첫 획득 시 지연 Prewarm** 추가. 기존 시그니처는 바꾸지 않았다.
* `AOngseongRamActor`가 `IPoolableActorInterface`를 구현하고 `Ram_ActorPool`에서 획득·반환된다.
* Pool 크기(테스트 레벨): 검병 9 · 궁병 8 · 화살 24 · 충차 2, 전부 `bAllowPoolExpansion=false`

## 5. 진행 로그

`LogOngseong` 카테고리를 추가했다. Display는 방어 시작/성공/실패, 인구 충원 완료, 충차 전진·타격.
Verbose는 개별 스폰·처치, 총통 표적 선택과 포탄 착탄이다.

---

# 변경 파일

## Core / Shared

* `Source/SuwonSiegeContestVR/Public|Private/Core/Debug/DebugFreeCameraPawn.h/.cpp` (신규)
* `Source/SuwonSiegeContestVR/Public|Private/Core/Debug/DebugFreeCameraGameMode.h/.cpp` (신규)
* `Source/SuwonSiegeContestVR/Public|Private/Gameplay/Combat/CombatFXLibrary.h/.cpp` (신규)
* `Source/SuwonSiegeContestVR/Public|Private/Gameplay/Pooling/ActorPool.h/.cpp`
* `Source/SuwonSiegeContestVR/SuwonSiegeContestVR.Build.cs` (`Niagara` 추가)

## GF_OngseongCrossbow

* `Public|Private/GF_OngseongCrossbow.h/.cpp` (`LogOngseong`)
* `Public|Private/Ongseong/OngseongEnemyWaveManager.h/.cpp`
* `Public|Private/Ongseong/OngseongDefenseScenarioManager.h/.cpp`
* `Public|Private/Ongseong/OngseongRamActor.h/.cpp`
* `Public|Private/Ongseong/ChongtongCannonActor.h/.cpp`, `ChongtongProjectileActor.h/.cpp`
* `Public|Private/Ongseong/OngseongNarrationComponent.h/.cpp`
* `Private/Tests/OngseongEnemyWaveManagerTests.cpp`, `Private/Tests/OngseongDefenseTests.cpp`

## 에셋

* `/GF_OngseongCrossbow/Blueprints/Debug/BP_OngseongCombatTestGameMode` (신규)
* `/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest` (신규 · `LV_Ongseong` 복제본)
  * GameMode Override, `bReturnToMainOnSuccess=false`, `RamPool` 연결
  * `Ram_ActorPool`, `Ram_SpawnPoint`(성문에서 3000cm), `DebugCamera_PlayerStart` 배치
  * Pool 크기 조정

## 스크립트 / 문서

* `Scripts/CreateOngseongCombatTestMode.py` (신규 · 위 에셋 생성/설정, 멱등)
* `Scripts/InspectOngseongCombatTest.py` (신규 · 레벨 거리·Pool·NavMesh 점검)
* `docs/OngseongCrossbow/ARCHITECTURE.md`, `STATUS.md`, `plans/2026-08-24_*.md` 3종

---

# 주요 결정 사항

1. **비VR GameMode는 Core에 둔다.** 신기전·공심돈도 같은 검증이 필요하고, 이 클래스는 어떤 Game Feature도
   참조하지 않는다. 옹성 전용 설정은 자식 Blueprint와 테스트 레벨에만 존재한다.
2. **스포너 클래스 이름은 유지한다.** 레벨 배치 인스턴스와 Blueprint 참조가 걸려 있어 Rename은
   불필요한 재저장을 유발한다(`CLAUDE.md` 14절).
3. **자유 카메라 입력은 폴링으로 처리한다.** Enhanced Input 전용 설정에서 엔진 기본 축 바인딩이
   동작하지 않을 위험을 설계로 제거했다.
4. **Pool은 첫 획득 시 지연 Prewarm한다.** Actor `BeginPlay` 순서는 보장되지 않는다.
5. **충차 이동은 Sweep을 쓰지 않는다.** 정해진 경로를 따라가는 연출 오브젝트이고 피해는 `ImpactGate`에서
   명시적으로 적용하므로, Sweep은 지형·성문에 끼어 전진을 멈추게 할 뿐이다.

---

# 테스트 결과

## 빌드

* `SuwonSiegeContestVR Win64 Development`: 성공
* `SuwonSiegeContestVREditor Win64 Development`: 성공

## Automation

`SuwonSiegeContestVR` 전체 **24/24 통과** (exit code 0). 갱신한 테스트는 다음을 검증한다.

* `Ongseong.WaveManager.Configuration` — 15명·8/7 슬롯·5초 리스폰 기본값, Pool 고갈 시 인원 미증가,
  `ResetWave`의 예약 취소
* `Ongseong.Defense.Contracts` — 충차 파괴 → 성공 전이, 성공 후 성문 보고 무시, 재시도 시 새 충차 1대,
  성문 파괴 → 실패·스폰 중단·인구 정리

> **테스트 하네스 수정**: `AActor::ProcessEvent`는 `World->AreActorsInitialized()`가 false인 동안
> UFUNCTION 호출(다이내믹 델리게이트 포함)을 조용히 버린다. 테스트 월드에서
> `InitializeActorsForPlay` + `BeginPlay`를 호출하도록 고쳤다. 기존 반복 Wave 테스트가 한 번도
> 통과한 적이 없던 원인도 동일하다.

## 비VR 테스트 레벨 구동 (헤드리스 `-game -nullrhi`)

```text
LogLoad: Game class is 'BP_OngseongCombatTestGameMode_C'
LogOngseong: Defense started. Ram=BP_OngseongRam_C_2, enemy slots=15, time limit=off
LogOngseong: Enemy spawning started: 8 swordsmen + 7 archers, respawn 5.0s (+/-1.5).
LogOngseong: Ongseong population filled: 15 living enemies.
LogOngseong: Ram advancing toward BP_OngseongGate_C_0 at 35 cm/s.
LogOngseong: Ram struck BP_OngseongGate_C_0 for 75 damage.   (13회)
LogOngseong: Warning: Defense failed (GateDestroyed).
```

* VR Pawn 생성 0개, 레벨 정상 로드
* 적 15명 충원 및 유지, `LogActorPool` 고갈 경고 0건
* 충차가 3000cm를 35 cm/s로 접근해 성문을 반복 타격하고, 성문 Health 소진 시 실패 전이까지 동작

## 구동 중 발견해 고친 문제

| 증상 | 원인 | 수정 |
|---|---|---|
| 방어가 시작되지 않고 `Ram_ActorPool exhausted (total 0)` | Pool보다 시나리오가 먼저 `BeginPlay` | `AcquireActor`의 지연 Prewarm |
| 충차가 5분간 전혀 움직이지 않음 | `SetActorLocation` Sweep이 지형·성문에 끼임 | Sweep 비활성 |
| 충차 접근이 즉시 끝남 | `RamSpawnPoint` 미설정 → 성문 180cm 지점에서 출발 | 테스트 레벨에 `Ram_SpawnPoint`(3000cm) 배치 |

---

# 남은 문제

## 1. 아군 총통이 적을 전혀 표적으로 삼지 못한다 (기존 레벨 구성 문제)

5분 구동 동안 처치 0건이며, 로그는 계속 다음과 같다.

```text
LogOngseong: Verbose: BP_AllyChongtong_C_0 has no target to fire at.
```

측정한 레벨 좌표는 다음과 같다.

| 대상 | 위치 | 비고 |
|---|---|---|
| 성문 `Ongseong_GateObjective` | (150, 100, 0) | |
| 적 스폰 `Ongseong_WaveManager` | (0, 0, 0) | **성문에서 180cm** — 사실상 성문 위 |
| 아군 총통 3기 | (±3000, 2500~5100, **600**) | 성벽 위 |
| 퇴각 지점 | (150, -3900, 0) | 적의 진입/퇴각 방향은 −Y |
| NavMeshBoundsVolume | 중심 (0, 3950, −90), 범위 (2000, 4000, 100) | y ≈ −50 ~ 7950 (성 안쪽만) |

`UCombatTargetingComponent::FindVisibleHostileTargets`는 사거리(12000) 안이라도 **Line of Sight가
막히면 표적에서 제외**한다. 적이 성문 바로 앞 지면에 있고 총통은 성벽 위에 있어 구조물이 시야를 막는다.
`ENEMY_ARCHER_BEHAVIOR_TREE_SETUP.md`와 `2026-08-20_CONTENT_LINKING_RESULT.md`에 이미 기록된
"시야 차단"·"발사선이 지형에 먼저 닿음" 이슈와 같은 계열이며, 이번 변경으로 생긴 문제가 아니다.

해결하려면 레벨 작업이 필요하다.

* 적 스폰 지점을 성문 바깥(−Y 방향, 충차와 같은 접근로)으로 옮기고
* NavMeshBoundsVolume을 그 통로까지 확장한 뒤 Navigation을 다시 빌드해야 한다.

두 변경 모두 실제 체험의 전투 구도를 바꾸므로 임의로 수행하지 않았다.

## 2. 그 밖의 남은 작업

* 사람이 PIE로 열어 자유 카메라 조작감과 시야, 전투 연출 육안 확인
* `SC_OngseongCombat` Sound Concurrency 에셋 생성 후 `ExplosionSoundConcurrency` / `CombatSoundConcurrency` 연결
* 본편 `LV_Ongseong`에 Pool 크기·`Ram_ActorPool`·`Ram_SpawnPoint`를 반영할지 결정
* Android 실기기에서 동시 15명 기준 `stat unit`, Niagara 풀 재사용 실측
