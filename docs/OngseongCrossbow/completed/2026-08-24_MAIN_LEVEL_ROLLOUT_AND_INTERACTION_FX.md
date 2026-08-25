# 본편 레벨 반영 · 충차 속도 · Sound Concurrency · 인터랙션 발광

**완료일**: 2026-08-24
**계획**: `plans/2026-08-24_MAIN_LEVEL_ROLLOUT_AND_INTERACTION_FX.md`

---

# 작업

사용자 지시 6건.

1. 테스트 레벨에서만 검증된 전투 배치를 본편 `LV_Ongseong`에 적용
2. 충차 접근 시간을 약 3분으로 조정
3. `SC_OngseongCombat` Sound Concurrency 생성 및 연결
4. 임시 파티클을 어울리는 에셋으로 교체
5. 플레이어 이동 정책을 "성벽 위 고정"으로 확정하고 구현
6. 인터랙션 구현 여부 점검 및 인터랙션 지점 발광/파티클 추가

---

# 구현 내용

## 1. 본편 레벨 반영

`Scripts/ApplyOngseongCombatLayout.py`로 적용했다. 헤드리스 실행 결과:

| 대상 | 변경 |
|---|---|
| `Ongseong_WaveManager` | (0, 0, 0) → **(150, 7800, 98)**, 성문을 향해 yaw -90 |
| `Ongseong_RamSpawnPoint` | → **(150, 8350, 243)** |
| `Ongseong_RetreatPoint` | → **(150, 8900, 98)** |
| 총통 4문 | 포구가 성벽 바깥을 향하던 4문 모두 yaw +180 |
| `Enemy_ActorPool` | 크기 9, 확장 비활성 |
| `Archer_ActorPool` | 크기 8, 확장 비활성 |
| `Ongseong_RangedProjectilePool` | 크기 24, 확장 비활성 |
| `Ram_ActorPool` | **신규 생성**, `BP_OngseongRam` 2개, 확장 비활성 |
| `Ongseong_DefenseScenario` | RamPool·RamSpawnPoint·RetreatPoint 연결 |
| `NavMeshBoundsVolume` | 범위 (2000,4000,100) → **(3600,4600,400)**, 중심 (250, 4580, 260)으로 이동 후 재빌드 |

레벨 백업: `Saved/CodexBackups/2026-08-24_OngseongMainLevelRollout/LV_Ongseong.umap`

## 2. 충차 접근 시간 ≈ 3분

`AOngseongRamActor::MoveSpeed` 35 → **42 cm/s**.

스폰 (150, 8350)에서 대기 지점(성문 800cm 앞)까지 **7,454cm**, 42cm/s로 **177초 = 2분 57초**.
스크립트가 실행 시 이 값을 계산해 로그로 남긴다.

## 3. Sound Concurrency

`/GF_OngseongCrossbow/Asset/Sound/SC_OngseongCombat` 생성.
동시 8음, `StopFarthestThenOldest`, VolumeScale 0.85.

연결 대상 5종: `BP_ChongtongProjectile`(Explosion), `BP_ChongtongCannon`·`BP_PlayableChongtong`·
`BP_AllyChongtong`(Combat), `BP_OngseongCrossbow`(Fire).

## 4. 파티클 교체

| 용도 | 이전 | 현재 |
|---|---|---|
| 총통 발사 | `/Niagara/.../SimpleExplosion` (엔진 템플릿) | `/Game/NiagaraExamples/FX_Weapons/MuzzleFlashes/NS_MuzzleFlash` |
| 포탄 폭발 | `/Niagara/.../SimpleExplosion` | `/Game/NiagaraExamples/FX_Explosions/NS_Dirt_Explosion_Medium` |
| 쇠뇌 발사 | 없음 | `/Game/NiagaraExamples/FX_Sparks/NS_Spark_Burst` (신규) |
| 장전 성공 | `/Game/NiagaraExamples/FX_PickUp/NS_Pickup_Success` | 유지 |
| 인터랙션 프롬프트 | 없음 | `/Game/NiagaraExamples/FX_PickUp/NS_Pickup_Idle` (신규) |

모두 프로젝트에 이미 포함된 `NiagaraExamples` 팩 자산이다. 최종 아트로 교체할 때는
Blueprint 속성만 바꾸면 되고 코드 수정은 필요 없다.

## 5. 플레이어 성벽 고정

* Core: `AVRPlayerPawn::bEnableTeleport` 추가, `SetLocomotionEnabled(bMove, bTeleport)` 추가.
  텔레포트 입력 3개 진입점에 게이트를 넣었고, 마운트 상호작용 복원값도 함께 갱신한다.
* Feature: `AOngseongDefenseScenarioManager::bLockPlayerToBattlement`(기본 true)가
  BeginPlay에서 이동·텔레포트를 모두 잠근다. 스냅 턴과 그랩은 그대로 동작한다.
* 비VR 테스트 GameMode의 자유 카메라 폰은 `AVRPlayerPawn`이 아니므로 영향을 받지 않는다.

`STATUS.md` 4.6의 미확정 항목이 이것으로 확정됐다.

## 6. 인터랙션 점검 및 발광

### 신규 Core 컴포넌트

`UInteractionHighlightComponent` (`Source/.../Core/VR/InteractionHighlightComponent.*`)

* Overlay Material(`SetOverlayMaterial`)로 **오브젝트 자체를 형태 그대로 발광**시킨다
* 같은 위치에 루프 Niagara를 켜서 멀리서도 보이게 한다
* 색은 MID 파라미터라 잡기=파랑, 양손 파지=호박색으로 구분한다
* Pool 반환(`EndPlay`) 시 발광을 반드시 끈다

Material: `/Game/Core/VR/Interaction/M_InteractionHighlight` (Unlit·Additive·Two-sided, Fresnel 림)
+ `MI_InteractionHighlight_Blue` / `MI_InteractionHighlight_Amber`.

Core에 두었으므로 신기전·공심돈·거중기에서 그대로 재사용할 수 있다.

### 인터랙션 목록과 점검 결과

`Scripts/AuditOngseongInteractions.py` 실행 결과 기준.

| # | 인터랙션 | 구동 방식 | 구현 | 시각 피드백 |
|---|---|---|---|---|
| 1 | 화약 집기 | `BP_ChongtongPowder` + Core `GrabPoint` | O | **파란 발광(신규)** |
| 2 | 화약 총통에 넣기 | 근접 30cm 판정 | O | Niagara + 사운드 + 단계 텍스트/조명 |
| 3 | 쑤시개 집기 | `BP_ChongtongRammer` + `GrabPoint` | O | **파란 발광(신규)** |
| 4 | 쑤시개 3회 왕복 | 삽입 30cm / 이탈 65cm | O | 회차별 피치 상승 + 진행 표시 |
| 5 | 대포알 집기 | `BP_ChongtongCannonball` + `GrabPoint` | O | **파란 발광(신규)** |
| 6 | 대포알 장전 | 근접 판정 | O | Niagara + 사운드 |
| 7 | 총통 양손 조준 | `UChongtongAimGripComponent` | O | **호박색 프롬프트(신규)** |
| 8 | 총통 발사(양손 트리거) | `TryFirePlayer` | O | **Muzzle Flash(교체)** + 사운드 |
| 9 | 쇠뇌 양손 파지 | `UOngseongCrossbowGripComponent` | O | **파란 프롬프트(신규)** |
| 10 | 쇠뇌 발사 | 트리거 | O | **Spark Burst(신규)** + 사운드 |
| 11 | 이동/텔레포트 | Core VR Pawn | O | 이번 작업으로 **의도적으로 비활성** |

발광은 **지금 필요한 하나만** 켜진다. 상태가 `NeedsPowder`면 화약만, `NeedsRamming`이면
쑤시개만, `NeedsCannonball`이면 대포알만, `ReadyToAim`이면 총통 그립만 빛난다.

---

# 변경 파일

```text
Source/SuwonSiegeContestVR/Public/Core/VR/InteractionHighlightComponent.h      (신규)
Source/SuwonSiegeContestVR/Private/Core/VR/InteractionHighlightComponent.cpp   (신규)
Source/SuwonSiegeContestVR/Public|Private/Core/VR/VRPlayerPawn.*
Source/SuwonSiegeContestVR/Public|Private/Gameplay/Combat/HealthComponent.*    (SetMaxHealth 추가)
Plugins/.../Ongseong/ChongtongLoadingItemActor.*
Plugins/.../Ongseong/ChongtongCannonActor.*
Plugins/.../Ongseong/ChongtongProjectileActor.cpp
Plugins/.../Ongseong/OngseongCrossbowActor.*
Plugins/.../Ongseong/OngseongRamActor.*
Plugins/.../Ongseong/OngseongDefenseScenarioManager.*
Plugins/.../Private/Tests/OngseongInteractionPromptTests.cpp                   (신규)
Scripts/CreateOngseongInteractionFX.py                                        (신규)
Scripts/ApplyOngseongCombatLayout.py                                          (신규)
Scripts/AuditOngseongInteractions.py                                          (신규)
Content/Core/VR/Interaction/M_InteractionHighlight.uasset                      (신규)
Content/Core/VR/Interaction/MI_InteractionHighlight_Blue.uasset                (신규)
Content/Core/VR/Interaction/MI_InteractionHighlight_Amber.uasset               (신규)
Plugins/.../Content/Asset/Sound/SC_OngseongCombat.uasset                       (신규)
Plugins/.../Content/Maps/LV_Ongseong.umap
Plugins/.../Content/Blueprints/BP_ChongtongProjectile|Cannon|PlayableChongtong|AllyChongtong|OngseongCrossbow.uasset
```

---

# 주요 결정 사항

* **하이라이트는 Core에 둔다.** 다른 체험도 같은 어포던스가 필요하고, Core는 어떤 Feature도
  참조하지 않는다. Feature는 켜고 끄기만 한다.
* **Overlay Material + Niagara 2중 구성.** 발광만으로는 시야 밖에서 안 보이고, 파티클만으로는
  "무엇을" 잡으라는 것인지 모호하다.
* **충차 체력 100 → 1000.** 아래 "구동 중 발견해 고친 문제" 참조.
* **레벨에 배치된 여분 충차는 삭제하지 않았다.** 다른 개발자의 레벨 콘텐츠이므로 보고만 한다.

---

# 테스트 결과

| 검증 | 결과 |
|---|---|
| `SuwonSiegeContestVREditor` Win64 Development | 성공 |
| `SuwonSiegeContestVR` Win64 Development | 성공 |
| `Automation RunTests SuwonSiegeContestVR` | **25/25 성공, 실패 0** (신규 `Ongseong.Interaction.Prompts` 포함) |
| `LV_Ongseong` 헤드리스 구동 | 아래 로그 |

```text
LogOngseong: Display: Player locomotion locked to the battlement post.
LogOngseong: Display: Ram advancing toward BP_OngseongGate_C_0 at 42 cm/s.
LogOngseong: Display: Enemy spawning started at V(X=150, Y=7800, Z=98): 8 swordsmen + 7 archers
LogOngseong: Display: Spawn point is on the navmesh (projected to V(X=150, Y=7800, Z=10)).
LogOngseong: Display: Defense started. Ram=BP_OngseongRam_C_2, enemy slots=15, time limit=off
LogOngseong: Display: Defense succeeded: the ram was destroyed after 92 enemies were defeated.
```

본편 레벨에서 **적 스폰 → 총통 교전 → 충차 접근 → 충차 파괴 클리어**가 처음으로 성립했다.
적용 전 같은 레벨에서는 스포너가 성문 위에 있고 총통이 성벽을 쏘아 교전 자체가 되지 않았다.

## 구동 중 발견해 고친 문제

**충차가 6초 만에 파괴되어 체험이 끝났다.**

충차 체력이 기본값 100인데 총통 포탄 1발이 직격 40 + 범위 80 = 120이다.
배치를 고쳐 총통이 통로를 보게 되자마자 첫 일제사격에서 충차가 즉사했다.
성문과 같은 **1000**으로 올렸다(`UHealthComponent::SetMaxHealth` 신설).

재구동 결과: 충차는 직격 7발 + 다수의 범위 피해를 받으며 **96초 생존**했다.

---

# 남은 문제

## 1. 본편 레벨에 목표가 아닌 여분 충차가 있다 — 레벨 담당자 확인 필요

`BP_OngseongRam_C_1`이 성 **안쪽** (1033, 2354, 290)에 배치되어 있다.
시나리오는 Pool에서 얻은 `BP_OngseongRam_C_2`를 목표로 쓰므로 이 액터는 목표가 아니지만,
적 진영 파괴 가능 액터라서 **아군 총통 사격 75발 중 13발(17%)을 여기에 낭비했다.**

Pool 도입 이전에 배치된 잔재로 보인다. 제거 또는 성 바깥 연출용으로 이동이 필요하다.
임의 삭제는 하지 않았다.

## 2. 아군 총통이 목표 충차를 대신 파괴한다 — 설계 판단 필요

무인 구동에서 아군 총통 3문이 96초 만에 충차를 파괴했다. 즉 플레이어가 아무것도 하지 않아도
체험이 클리어된다. 충차가 적 대열 한가운데로 전진하므로 범위 피해가 계속 누적되는 것이 원인이다.

선택지:

* (a) 아군 총통이 목표 충차를 표적에서 제외 (플레이어 전용 목표로 확정)
* (b) 충차 체력을 더 올려 3분 접근을 모두 보여줌
* (c) 현행 유지 — 아군 지원 사격도 연출의 일부로 인정

(a)를 택하면 `AChongtongCannonActor`에 "목표 액터 제외" 옵션이 필요하다.

## 3. 쇠뇌 탄약 보충 수단이 없다

`MaxAmmo = 12`, `RefillAmmo()`는 구현되어 있으나 **호출하는 곳이 없다.**
12발을 다 쓰면 HUD에 "쇠뇌 화살을 보충하십시오"가 뜬 뒤 영구히 사용 불가가 된다.

임시 회피책은 `MaxAmmo = 0`(무한)이며, 정식 해결은 화살통 상호작용 배치다. 어느 쪽이든
콘텐츠 설계 판단이 필요해 임의로 넣지 않았다.

## 4. 그대로 남는 기존 항목

* 사람이 PIE/HMD로 열어 발광·파티클·전투 연출을 육안 확인 — **헤드리스로는 검증 불가**
* Android 실기기 성능 실측 (`stat unit`, `stat NiagaraOverview`)
* 임시 메시·애니메이션·SFX·나레이션 녹음 교체
* PlayerPhone 옹성 기능 (`STATUS.md` 4.7) 미정
* NavMesh가 통로 북쪽 끝(y ≈ 7961)까지만 생성되는 문제
