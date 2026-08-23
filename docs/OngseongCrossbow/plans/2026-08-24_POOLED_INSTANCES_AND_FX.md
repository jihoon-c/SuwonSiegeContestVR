# 생성/소멸 인스턴스 및 파티클 오브젝트 풀링

**작성일**: 2026-08-24 · **대상 계층**: Shared Gameplay (공용 계약 추가) + `GF_OngseongCrossbow`
**상태**: `Implemented (Sound Concurrency 에셋·실측 대기)` — 하단 "진행 상태" 참조.

---

# 목적

옹성 체험에서 런타임에 생성/소멸하는 모든 인스턴스를 오브젝트 풀로 처리해 Android 스탠드얼론
VR의 스폰 스파이크와 GC 부하를 줄인다. 적 상시 15명 유지(`2026-08-24_SUSTAINED_ENEMY_POPULATION`)의
전제 조건이다.

대상: 적 병사, 궁병 화살, 총통 포탄, 쇠뇌 볼트, 충차, 포탄 폭발 파티클.

---

# 현재 상태 (실제 조사 결과)

| 대상 | 풀링 여부 | 확인 내용 |
|---|---|---|
| 적 검병/궁병 | 풀링됨 | `AOngseongEnemyWaveManager`가 `AActorPool::AcquireActor/ReleaseActor` 사용. `Enemy_ActorPool` 고정 8, 궁병은 `Ongseong.ArcherPool` 태그 Pool |
| 총통 포탄 | 풀링됨 | `AChongtongCannonActor::Fire`가 `ProjectilePool`에서 획득. 레벨 Pool 16 |
| 쇠뇌 볼트 | 풀링됨 | `AOngseongCrossbowActor`의 `BoltPool` 24 고정 |
| 궁병 화살 | 풀링됨 | `UOngseongArcherCombatComponent`가 `Ongseong.ArrowPool` 태그 Pool 사용 |
| 투사체 반환 | 구현됨 | `AGameplayProjectileActor`가 충돌·수명 만료 시 소유 Pool로 반환 |
| **충차** | **미풀링** | `OngseongDefenseScenarioManager.cpp:78,92,187` — `SpawnActor` / `Destroy()` |
| **폭발 파티클** | **미풀링** | `ChongtongProjectileActor.cpp:39`, `ChongtongCannonActor.cpp:340` — `UNiagaraFunctionLibrary::SpawnSystemAtLocation` 기본 인자(풀링 미지정) |
| **전투 사운드** | **미제한** | `UGameplayStatics::PlaySoundAtLocation`. 동시 재생 상한(Sound Concurrency) 없음 |
| Pool 고갈 처리 | **조용히 실패** | `AActorPool::AcquireActor`가 `nullptr` 반환. 경고 로그·이벤트 없음 |

즉 Actor 풀링 골격은 이미 있고, **파티클·충차·Pool 크기·고갈 가시성**이 비어 있다.

---

# 구현 범위

## 1. Shared: Niagara 풀 재생 래퍼

`Source/SuwonSiegeContestVR/.../Gameplay/Combat/CombatFXLibrary.h`에 `UCombatFXLibrary`
(BlueprintFunctionLibrary)를 추가한다.

```text
SpawnPooledSystemAtLocation(WorldContext, System, Location, Rotation, Scale)
    → UNiagaraFunctionLibrary::SpawnSystemAtLocation(..., bAutoDestroy=true, bAutoActivate=true,
                                                     ENCPoolMethod::AutoRelease, bPreCullCheck=true)
PlayPooledSoundAtLocation(WorldContext, Sound, Location, Concurrency)
```

* 엔진 Niagara Component Pool을 사용하므로 별도 Actor Pool을 만들지 않는다.
* 무한 루프 시스템은 `AutoRelease`와 맞지 않으므로 원샷 이펙트에만 사용한다.
* Feature는 이 라이브러리만 호출하고 Niagara API를 직접 호출하지 않는다.

## 2. Shared: `AActorPool` 가시성 보강 (추가만, 기존 동작 변경 없음)

* Pool 고갈 시 `UE_LOG(LogTemp, Warning, ...)` 대신 전용 카테고리로 Pool 이름·클래스·요청 수를 남긴다.
* `OnPoolExhausted` 델리게이트 추가 (Blueprint에서 경고 UI/로그 연결 가능)
* `GetPooledActorClass()` 조회 추가

기존 `AcquireActor` / `ReleaseActor` / `PrewarmPool` 시그니처와 동작은 그대로 두어
신기전·공심돈에 영향이 없도록 한다.

## 3. Feature: 충차 풀링

* `AOngseongRamActor`가 `IPoolableActorInterface`를 구현한다.
  * `OnAcquiredFromPool`: Health 리셋, 상태 `Advancing`, 이동 목표 초기화
  * `OnReleasedToPool`: Tick 정지, 타이머 정리, 목표 해제
* 레벨에 `Ram_ActorPool`(크기 2, 자동 확장 비활성)을 배치한다.
* `AOngseongDefenseScenarioManager`의 `SpawnActor`/`Destroy` 경로를 Pool 획득/반환으로 교체한다.

## 4. Feature: 파티클·사운드 교체

* `AChongtongProjectileActor::HandleExplosion`의 폭발 FX/사운드
* `AChongtongCannonActor::PlayFeedback`의 장전·발사 FX/사운드
* 위 두 곳을 `UCombatFXLibrary` 호출로 교체하고, 폭발 사운드에 `SC_OngseongCombat`
  Sound Concurrency(동시 4개, 초과 시 가장 오래된 것 정지)를 지정한다.

## 5. Pool 크기 재산정

`2026-08-24_SUSTAINED_ENEMY_POPULATION`의 동시 15명 기준.

| Pool | 현재 | 변경 |
|---|---|---|
| 검병 (`Enemy_ActorPool`) | 8 | 9 (검병 슬롯 8 + 여유 1) |
| 궁병 (`Ongseong.ArcherPool`) | 태그 Pool | 8 (궁병 슬롯 7 + 여유 1) |
| 궁병 화살 (`Ongseong.ArrowPool`) | 미확정 | 24 |
| 총통 포탄 (`Projectile_ActorPool`) | 16 | 16 유지 |
| 쇠뇌 볼트 | 24 | 24 유지 |
| 충차 (`Ram_ActorPool`) | 없음 | 2 |

모든 Pool은 `bAllowPoolExpansion=false`를 유지한다. 상한을 넘기는 대신 스폰 실패를
경고로 드러내어 예산을 지킨다.

---

# 변경 예정 파일

* `Source/SuwonSiegeContestVR/Public/Gameplay/Combat/CombatFXLibrary.h` (신규)
* `Source/SuwonSiegeContestVR/Private/Gameplay/Combat/CombatFXLibrary.cpp` (신규)
* `Source/SuwonSiegeContestVR/Public/Gameplay/Pooling/ActorPool.h`
* `Source/SuwonSiegeContestVR/Private/Gameplay/Pooling/ActorPool.cpp`
* `Source/SuwonSiegeContestVR/SuwonSiegeContestVR.Build.cs` (Niagara 모듈 의존성 확인)
* `Plugins/.../Public/Ongseong/OngseongRamActor.h`, `.../Private/Ongseong/OngseongRamActor.cpp`
* `.../Private/Ongseong/OngseongDefenseScenarioManager.cpp`
* `.../Private/Ongseong/ChongtongProjectileActor.cpp`
* `.../Private/Ongseong/ChongtongCannonActor.cpp`
* `Plugins/.../Content/Maps/LV_Ongseong.umap` (Pool 배치·크기)
* `Plugins/.../Content/Audio/SC_OngseongCombat.uasset` (신규 Sound Concurrency)
* `Source/SuwonSiegeContestVR/Private/Tests/SharedGameplayTests.cpp` (Pool 고갈 계약)
* `docs/OngseongCrossbow/ARCHITECTURE.md`, `STATUS.md`, `docs/Main/Gameplay/STATUS.md`

---

# 구현 단계

1. Shared `UCombatFXLibrary`를 추가하고 Niagara 모듈 의존성을 확인한다.
2. `AActorPool`에 고갈 로그와 `OnPoolExhausted`를 추가한다. (기존 시그니처 유지)
3. 옹성의 Niagara/사운드 호출을 라이브러리 호출로 교체한다.
4. `AOngseongRamActor`에 Poolable 계약을 구현하고 시나리오 Manager의 Spawn/Destroy를 교체한다.
5. 레벨에 `Ram_ActorPool`을 배치하고 Pool 크기를 재산정 값으로 조정한다.
6. Sound Concurrency 에셋을 만들고 폭발/발사 사운드에 지정한다.
7. Automation Test에 Pool 고갈 계약과 충차 Pool 반환을 추가한다.

---

# 다른 Feature에 미치는 영향

* `AActorPool`과 `UCombatFXLibrary`는 Shared Gameplay다. 신기전·공심돈도 이후 같은 방식으로
  파티클을 풀링할 수 있다. **추가만 하고 기존 계약은 바꾸지 않으므로 즉시 영향은 없다.**
* Shared는 어떤 Game Feature도 참조하지 않는다. 옹성 전용 설정(Pool 크기, Concurrency 에셋)은
  Feature 콘텐츠에만 존재한다.
* `SuwonSiegeContestVR.Build.cs`에 Niagara 의존성을 추가하는 경우 전체 모듈 재빌드가 발생한다.
  다른 개발자와 타이밍을 조율한다.

---

# 검증 방법

* `SuwonSiegeContestVREditor Win64 Development` 빌드
* `SuwonSiegeContestVR` Automation 전체 통과 (Shared + Ongseong)
* `LV_Ongseong_CombatTest`에서 3분 PIE 후 확인
  * 런타임 중 `AOngseongRamActor` / 투사체 / 적 Actor의 `SpawnActor` 호출 0건
  * `stat unit`, `stat NiagaraOverview`로 폭발 다발 구간 프레임 스파이크 비교 (변경 전/후)
  * `fx.NiagaraComponentPool.Validation 1`로 풀 재사용 확인
  * Pool 고갈 경고 로그 0건 (발생 시 크기 재산정)
* Android 실측 프로파일링은 별도 범위로 남긴다.

---

# 위험 요소

* Niagara 풀 재사용은 시스템이 파티클 상태를 초기화하지 않으면 잔상이 남는다. 원샷 폭발 계열에만
  적용하고, 문제가 보이면 해당 시스템만 `ENCPoolMethod::None`으로 되돌린다.
* 현재 폭발 FX는 엔진 템플릿(`SimpleExplosion`)과 임시 사운드(`Fire_Cue`)다. 최종 아트 교체 시
  풀링 적합성을 다시 확인해야 한다.
* Pool 자동 확장을 끈 상태에서 크기를 잘못 잡으면 적이 스폰되지 않는다. 2단계의 고갈 로그를
  먼저 넣어 조용한 실패를 없앤다.

---

# 진행 상태 (2026-08-24)

## 구현 완료 (C++)

* Shared `UCombatFXLibrary` 신설 (`Gameplay/Combat/CombatFXLibrary.h/.cpp`)
  * `SpawnPooledSystemAtLocation`: `ENCPoolMethod::AutoRelease` + `bPreCullCheck`로 엔진 Niagara Component Pool 사용
  * `PlayPooledSoundAtLocation`: 선택적 `USoundConcurrency` 인자
  * `SuwonSiegeContestVR.Build.cs`에 `Niagara` 의존성 추가
* Shared `AActorPool` 가시성 보강 (기존 시그니처 유지)
  * `LogActorPool` 카테고리 추가, 고갈 시 Pool 이름·클래스·활성 수·확장 여부를 Warning으로 남긴다.
  * `OnPoolExhausted(Pool, ActiveCount)` 델리게이트, `GetExhaustedRequestCount()`, `GetPooledActorClass()` 추가
* 충차 풀링
  * `AOngseongRamActor`가 `IPoolableActorInterface` 구현 (획득 시 Health 리셋·표시 복원, 반환 시 정지·목표 해제)
  * `AOngseongDefenseScenarioManager`에 `RamPool` 추가. Pool이 지정되면 획득/반환하고, 없으면 기존 Spawn/Destroy로 동작한다(레벨 하위 호환).
* 파티클·사운드 교체
  * `AChongtongProjectileActor::HandleExplosion`, `AChongtongCannonActor::PlayFeedback`가 `UCombatFXLibrary`를 호출한다.
  * `ExplosionSoundConcurrency`, `CombatSoundConcurrency` 속성을 노출해 Concurrency 에셋을 데이터로 연결할 수 있다.

## 남은 작업 (에디터 필요)

* `SC_OngseongCombat` Sound Concurrency 에셋 생성 및 두 속성에 연결
* 레벨 Pool 크기 재산정과 `Ram_ActorPool` 배치 — `Scripts/CreateOngseongCombatTestMode.py`가 테스트 레벨에서 수행
* `stat unit` / `stat NiagaraOverview` / `fx.NiagaraComponentPool.Validation 1` 측정

## 검증 상태

* Game 타깃 빌드: **성공**
* Editor 타깃 빌드/Automation/PIE 측정: **보류** (Live Coding 활성)

---

# 진행 상태 갱신 (2026-08-24, 2차)

## 추가로 발견하고 고친 문제

비VR 테스트 레벨을 실제로 구동했을 때 다음 경고가 나오며 방어가 시작되지 않았다.

```text
LogActorPool: Warning: ActorPool_4 exhausted: BP_OngseongRam_C, active 0, total 0, expansion off.
```

Actor의 `BeginPlay` 순서는 보장되지 않는다. 시나리오 매니저가 Pool보다 먼저 `BeginPlay`를 실행하면
Pool은 아직 Prewarm되지 않은 상태이고, 요청은 조용히 실패한다.

**수정**: `AActorPool::AcquireActor`가 아직 Prewarm되지 않았다면 그 자리에서 Prewarm한다(`bHasPrewarmed`).
`PrewarmPool()`은 여전히 `BeginPlay`에서 호출되며 중복 생성은 없다. 이 문제는 이번에 추가한
**Pool 고갈 경고 로그 덕분에 즉시 특정**할 수 있었다.

## 레벨 반영 완료

| Pool | 크기 |
|---|---|
| `Enemy_ActorPool` (검병) | 9 |
| `Archer_ActorPool` | 8 |
| `Ongseong_RangedProjectilePool` (화살) | 24 |
| `Ram_ActorPool` | 2 |

모두 `bAllowPoolExpansion=false`이며 `LV_Ongseong_CombatTest`에 저장했다.

## 검증 결과

* Automation 24/24 통과 (`Gameplay.Pooling.CapacityInvariant` 포함)
* 헤드리스 구동 3분: `LogActorPool` 고갈 경고 0건, 적 15명 유지
* 충차는 Pool에서 획득되어 전진 (`Ram advancing ... at 35 cm/s`)

## 남은 작업

* `SC_OngseongCombat` Sound Concurrency 에셋 생성 후 `ExplosionSoundConcurrency` / `CombatSoundConcurrency`에 연결
* `stat unit` · `stat NiagaraOverview` · `fx.NiagaraComponentPool.Validation 1` 실측 (Android 포함)
* 본편 `LV_Ongseong`의 Pool 크기 재산정 (현재는 테스트 레벨만 반영)
