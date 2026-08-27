# 옹성 가시성·검병 배회·스폰 밀도 수정

> 상태: 코드 반영 완료, **빌드/PIE 검증 미실행**.
> 작업 시점에 Unreal Editor의 Live Coding이 활성이라 `Build.bat`이 거부되었다.
> 에디터를 닫고 다시 빌드한 뒤 아래 "검증 절차"를 수행해야 한다.

## 1. 충차 붉은 하이라이트 깜빡임

`UInteractionHighlightComponent`(Core)에 펄스 기능을 추가했다.

* `SetHighlightPulse(bool, float PulsesPerSecond)` / `bPulseHighlight` / `PulseMinIntensityScale` /
  `PulseMaxIntensityScale`.
* 펄스는 코사인 파형으로 오버레이 머티리얼의 `Intensity` 스칼라를 변조한다.
  헤드셋에서 사각파 점멸은 스트로브처럼 보이므로 사각파를 쓰지 않는다.
* Tick은 펄스가 실제로 도는 동안에만 켜진다(`bStartWithTickEnabled = false`).
* 기본값은 `false`이므로 기존 grab prompt 하이라이트 동작은 그대로다.

`AOngseongRamActor`에 `bPulseVisibilityHighlight`(기본 true), `VisibilityHighlightPulsesPerSecond`
(기본 1.0)를 추가하고 `BeginPlay`/`ActivateRam`에서 적용한다.

## 2. 궁병 공격 포지션 순간이동

`ArcherApproachRadius` 기본값 `100` → `450`.
수용 반경이 좁으면 path following이 마지막 보정을 한 프레임에 처리해 슬롯으로 붙는 것처럼 보인다.

## 3. 검병 호위 로직 삭제 → 성 내부 배회

요청 3번("호위 반경 확대")과 요청 8번("호위 로직 삭제")이 충돌하므로 **8번을 채택**했다.
호위 반경 대신 배회 반경(`SwordsmanRoamRadius`, 기본 3000)이 그 역할을 대신한다.

삭제한 것:
`SwordsmanFollowSpeed`, `SwordsmanWanderSpeed`, `SwordsmanFollowTriggerDistance`,
`SwordsmanSettleDistance`, `SwordsmanEscortSpacing`, `SwordsmanWanderRadius`,
`SwordsmanFollowTargetRefreshDistance`, `SwordsmanEscortSectors`, `SwordsmenFollowingRam`,
`AssignSwordsmanEscortSector`, `BuildSwordsmanDestination`, `ProjectEscortDestinationToNavigation`,
`ApplySwordsmanEscortBehavior`, `UpdateSwordsmanEscortBehavior`.

추가한 것:
`SwordsmanRoamSpeed`(140), `SwordsmanRoamRadius`(3000), `SwordsmanRoamAnchor`,
`SwordsmanRoamArrivalDistance`(200), `SwordsmanRoamIntervalMin/Max`(3~7초),
`ApplySwordsmanRoamBehavior`, `BuildSwordsmanRoamDestination`, `ResolveSwordsmanRoamAnchor`,
`UpdateSwordsmanRoamBehavior`.

검병은 `GetRandomReachablePointInRadius`로 **도달 가능한** NavMesh 지점만 고른다.
도착하거나 3~7초가 지나면 다음 지점을 다시 고른다.

배회 중심 우선순위: `SwordsmanRoamAnchor` → `Ongseong.SwordsmanRoamAnchor` 태그 Actor →
`ObjectiveTarget` → WaveManager 자신.

`SetArcherEscortTarget`은 남긴다. 이제 슬롯을 못 잡은 **잉여 궁병**에게만 쓰인다.

## 4. 총통 포탄 착탄 파티클

원인을 에디터 없이 확정하지 못했으므로 세 가지를 함께 처리했다.

* `UCombatFXLibrary::SpawnPooledSystemAtLocation`에 `bPreCullCheck` 인자를 추가(기본 `true` 유지).
  포탄 폭발은 `false`로 호출한다. Niagara 시스템 자체의 cull 설정이 스폰을 조용히 버리는 경로를 막는다.
* `ExplosionEffectScale` 기본값 `1.0` → `2.0`.
  (처음 `6.0`으로 올렸다가 되돌렸다. 샘플 시스템의 Light Renderer 반경이 함께 커져
  지형이 초록으로 물드는 문제를 키웠다. `docs/OngseongCrossbow/2026-08-27_NIAGARA_GREEN_TINT_DIAGNOSIS.md` 참고.) 기본 임팩트 시스템은 총알용이라 1.0에서는
  포탄 착탄이 거의 보이지 않는다.
* 스폰이 실패하면 `LogOngseong` Warning으로 액터 이름·위치·`ExplosionEffect`를 남긴다.

`BP_ChongtongProjectile`은 `ExplosionEffect`를 재정의하지 않으므로 C++ 기본값
`NS_Impact_Concrete`를 상속한다. 위 Warning이 뜬다면 그 값이 실제로 비어 있는 것이다.

## 4-1. 적병 머리에서 폭발하는 문제

포탄이 병사에 스치면 `Hit.ImpactPoint`가 머리 높이라 그 자리에서 터졌다.

* `AChongtongProjectileActor::FindGroundedEffectLocation()`을 추가했다. 착탄점에서 아래로
  `GroundTraceDistance`(기본 600cm)만큼 **WorldStatic만** 트레이스해 바닥을 찾고,
  `GroundEffectHeightOffset`(기본 15cm) 띄운 지점에서 파티클과 폭발음을 재생한다.
  Pawn을 트레이스에서 제외하지 않으면 옆 병사 머리 위에서 터지므로 반드시 WorldStatic 전용이다.
* 바닥을 못 찾으면(성벽 측면 명중, 공중 폭발) 원래 착탄점을 그대로 쓴다.
* **데미지는 건드리지 않았다.** 직격 데미지는 기존대로 `HandleProjectileHit`에서,
  범위 데미지는 기존대로 실제 착탄점(`Location`) 기준 `ExplosionRadius` 구체로 계산한다.
  연출 위치만 바닥으로 내린다.
* 토글: `bGroundExplosionEffect`(기본 true), `GroundTraceDistance`, `GroundEffectHeightOffset`.
  `BP_ChongtongProjectile` Class Defaults의 `Ongseong|Chongtong|Explosion`에 있다.

## 5. 총통 머즐 플래시 크기 조절 방법 (코드 변경 없음)

이미 에디터에 노출되어 있다.

1. `BP_ChongtongCannon`(또는 `BP_AllyChongtong` / `BP_PlayableChongtong`)을 연다.
2. **Class Defaults** → Details 검색창에 `Muzzle`.
3. `Ongseong|Chongtong|Feedback` 카테고리의 **Muzzle Effect Scale** 값을 올린다. (현재 기본 `2.0`)

레벨에 배치된 특정 총통 하나만 키우려면 그 액터를 선택해 Details 패널에서 같은 값을 바꾸면 된다.
`Muzzle Effect` 슬롯의 Niagara 시스템 자체를 교체하는 것도 같은 카테고리에서 가능하다.

## 6. 궁병 화살 projectile

두 가지 원인을 고쳤다.

* **화살이 보이지 않았다.** `BP_OngseongBolt`(`Ongseong.ArrowPool`의 Pooled Actor Class)에는
  StaticMesh가 배정된 적이 없다. `AOngseongBoltProjectileActor` 생성자에서
  `/GF_OngseongCrossbow/Asset/Prop/Arrow/bow_and_arrow/StaticMeshes/Arrow`를 기본 메시로 지정하고
  `BoltMeshScale` / `BoltMeshRotation`을 노출했다.
* **발사 자체가 멈출 수 있었다.** 반복 발사 타이머가 `BeginAttackAnimation` 안에서만 시작되므로,
  BT의 이동 완료 브랜치가 끝나지 않으면 궁병은 영원히 쏘지 않았다.
  이제 `ActivateCombat()`에서 타이머를 건다. `TryFireArrow()`는 사거리 밖이거나
  공격 중이면 아무것도 하지 않으므로 중복 발사는 없다.

가시성을 위해 화살에 붉은 펄스 하이라이트를 붙였다(`VisibilityHighlightColor`,
`VisibilityHighlightPulsesPerSecond` 기본 4.0). Niagara 마커는 끈다 —
6500 cm/s로 나는 화살에 루프 시스템을 매다는 것은 비용만 든다.
Pool에 반납될 때 하이라이트를 끄므로 대기 중인 화살이 Tick하지 않는다.

## 7. 병사 스폰 밀도

`AOngseongEnemyWaveManager` 기본값:

| 항목 | 이전 | 이후 |
|---|---|---|
| Maximum Spawned Enemy Soldiers | 15 | 40 |
| Swordsman Slots | 8 | 22 |
| Archer Slots | 7 | 18 |
| Respawn Delay | 5.0 | 2.0 |
| Respawn Delay Jitter | 1.5 | 0.75 |
| Initial Spawn Interval | 0.4 | 0.2 |

## 7-1. Pool 크기 자동 조정

레벨에 저장된 `InitialPoolSize`는 WaveManager의 슬롯 수를 따라가지 않는다. Pool이 마르면
`exhausted` Warning만 남기고 조용히 보충을 멈춘다.

* `AActorPool::EnsurePoolSize(int32)`를 추가했다. 줄이지는 않고 필요한 만큼만 늘린다.
  `bAllowPoolExpansion` 기본값(false)은 건드리지 않았다.
* `AOngseongEnemyWaveManager::ResizePoolsToSlotCounts()`가 BeginPlay에서 호출된다.
  * 전용 궁병 Pool이 있으면 `EnemyPool` = 검병 슬롯 + `EnemyPoolHeadroom`(6),
    `ArcherEnemyPool` = 궁병 슬롯 + headroom.
  * 전용 궁병 Pool이 없으면 `EnemyPool` = 최대 인원 + headroom.
  * `ArcherProjectilePool` = 궁병 슬롯 × `ArrowsPerArcher`(4).
* Headroom은 사망 연출 중이라 아직 반납되지 않은 액터를 위한 여유분이다.
* 끄려면 `bResizePoolsToSlotCounts`를 false로.

## 7-2. 검병이 충차 주변에 엉겨붙던 문제

원인 두 가지를 고쳤다.

1. **배회 중심이 목표물이었다.** `ResolveSwordsmanRoamAnchor()`의 폴백 순서에 `ObjectiveTarget`이
   있었는데, 그게 충차가 때리는 성문이다. 중심이 성문이면 반경 안 랜덤 지점도 성문 주변에 몰린다.
   `ResolveSwordsmanRoamCentre()`로 바꾸고 폴백을 다시 짰다:
   `SwordsmanRoamAnchor` → `Ongseong.SwordsmanRoamAnchor` 태그 →
   **`Ongseong.ArcherAttackPosition` TargetPoint들의 중심** → WaveManager 자신.
   `ObjectiveTarget`은 폴백에서 완전히 뺐다. 궁병 공격 포지션은 성벽 안쪽 NavMesh 위에 배치된 것이
   보장된 유일한 점 집합이라 성 내부 중심으로 안전하다.
2. **스폰 직후 성문으로 행군 명령이 걸려 있었다.** `Enemy->SetObjectiveTarget()`이 내부에서
   `MoveToCombatActor(ObjectiveTarget)`를 호출한다. 검병은 배회 전에 `StopCombatMovement()`로
   그 명령을 취소한다.

## 7-3. 검병 이동속도

`SwordsmanRoamSpeed` 기본값 `140` → `0`.
`0`은 "캐릭터의 `MaxWalkSpeed`를 건드리지 않는다"는 뜻이고, 궁병이 바로 그 상태다.
따라서 두 병종의 이동속도가 같아진다. 일부러 다르게 하고 싶을 때만 양수를 넣는다.

## 변경 파일

```text
Source/SuwonSiegeContestVR/Public/Core/VR/InteractionHighlightComponent.h
Source/SuwonSiegeContestVR/Private/Core/VR/InteractionHighlightComponent.cpp
Source/SuwonSiegeContestVR/Public/Gameplay/Pooling/ActorPool.h
Source/SuwonSiegeContestVR/Private/Gameplay/Pooling/ActorPool.cpp
Source/SuwonSiegeContestVR/Public/Gameplay/Combat/CombatFXLibrary.h
Source/SuwonSiegeContestVR/Private/Gameplay/Combat/CombatFXLibrary.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/OngseongRamActor.h
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Ongseong/OngseongRamActor.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/OngseongBoltProjectileActor.h
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Ongseong/OngseongBoltProjectileActor.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/ChongtongProjectileActor.h
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Ongseong/ChongtongProjectileActor.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Ongseong/OngseongArcherCombatComponent.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/OngseongEnemyWaveManager.h
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Ongseong/OngseongEnemyWaveManager.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Tests/OngseongEnemyWaveManagerTests.cpp
Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Tests/OngseongDefenseTests.cpp
```

## 다른 Feature에 미치는 영향

`UInteractionHighlightComponent`와 `UCombatFXLibrary`는 Core/Shared다.

* 하이라이트 펄스는 기본 `false`, `bPreCullCheck`는 기본 `true`이므로 기존 호출부 동작은 불변이다.
* Tick은 켜질 수 있게 바뀌었지만 펄스가 도는 동안에만 실행된다.

## 레벨/에디터에서 해야 할 일

1. **에디터를 닫고 다시 빌드한다.** (또는 에디터에서 `Ctrl+Alt+F11`)
2. `LV_Ongseong_CombatTest`의 `Ongseong_WaveManager`는 `MaxConcurrentEnemies`와 `ArcherSlots`를
   **인스턴스에서 재정의**하고 있다. 새 기본값이 적용되지 않으므로 Details에서 직접 올리거나
   해당 프로퍼티를 "Reset to Default" 해야 한다. `LV_Ongseong`은 재정의가 없어 새 기본값을 그대로 받는다.
3. Pool 크기는 이제 BeginPlay에서 자동으로 늘어난다(7-1). `LogActorPool`에 `grown to N` 로그가
   찍히는지 확인한다. `exhausted` Warning이 여전히 나오면 headroom을 올린다.
4. 배회 중심을 명시하려면 성 내부에 Actor를 하나 두고 Tag `Ongseong.SwordsmanRoamAnchor`를 붙인다.
   비워 두면 `ObjectiveTarget`이 중심이 된다. `SwordsmanRoamRadius`가 성 내부 NavMesh를 덮는지 확인한다.
5. `BP_OngseongBolt`를 열어 화살 메시의 크기·회전이 맞는지 보고 `BoltMeshScale` /
   `BoltMeshRotation`을 조정한다. 화살촉이 진행 방향을 향해야 한다.

## 검증 절차 (미실행)

* Editor 모듈 빌드 성공 여부
* `OngseongEnemyWaveManagerTests`, `OngseongDefenseTests` 통과 여부
* PIE: 충차 붉은 테두리가 초당 1회 밝기 왕복하는지
* PIE: 궁병이 공격 포지션에 걸어서 도착하는지(순간이동 없음)
* PIE: 검병이 성 내부 여러 지점으로 계속 이동하는지, 충차를 따라가지 않는지
* PIE: 포탄 착탄 지점에 파티클이 보이는지 / `LogOngseong` Warning이 뜨는지
* PIE: 포탄이 적병에 맞았을 때 폭발이 병사 발밑 지면에서 일어나는지, 그리고 그 병사가 여전히 죽는지
* PIE: 궁병 화살이 눈에 보이고 붉게 깜빡이며 날아가는지
* PIE: 동시 생존 적이 26명까지 차오르는지, Pool exhausted Warning이 없는지

## 남은 문제

* 전부 미검증. 특히 4번(착탄 파티클)은 근본 원인을 확정하지 못했다.
* 지면 트레이스는 지형이 `WorldStatic`이라고 가정한다. 옹성 바닥이 `WorldDynamic`으로 설정돼
  있다면 폭발이 착탄점에 그대로 남으므로 `FindGroundedEffectLocation`의 채널을 확인해야 한다.
* 착탄/머즐 Niagara의 초록 오염은 미해결. 샘플 시스템 복제 후 Light/Decal Renderer 제거가 필요하다.
  `docs/OngseongCrossbow/2026-08-27_NIAGARA_GREEN_TINT_DIAGNOSIS.md` 참고.
* `SwordsmanRoamRadius` 3000cm는 추정값이다. 실제 성 내부 크기에 맞춰 조정해야 한다.
* `AOngseongBoltProjectileActor`의 화살 메시 기본 Scale/Rotation은 추정값이다.
