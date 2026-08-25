# 쇠뇌 폐지 · 아군 총통 교전 규칙 · 충차/탄도/애니메이션 수정

**완료일**: 2026-08-25

---

# 작업

사용자 지시 9건 (2회에 나눠 접수).

| # | 지시 | 결과 |
|---|---|---|
| A1 | 성 안쪽 여분 충차 제거 | 완료 |
| A2 | 아군 총통 표적에서 충차 제외, 적 궁병과만 교전 | 완료 |
| A3 | 쇠뇌 레거시 로직 삭제 | 완료 |
| B1 | 충차 로테이션 90도 틀어짐 정상화 | 완료 (육안 확인 필요) |
| B2 | 총통 탄속을 낮추고 포물선 궤적으로 | 완료 |
| B3 | 아군 총통이 공격 시 표적을 바라보도록 회전 | 완료 (육안 확인 필요) |
| B4 | 아군 총통 공격 시 포구 화염 + 포격 사운드 | 완료 (육안 확인 필요) |
| B5 | 검병 돌진 시 Run 애니메이션이 첫 자세로 고정 | 원인 확정, 에셋 수정 완료. **ABP 노드 1건 수동 확인 필요** |
| B6 | 아군 총통 어택 슬롯 (적 궁병 최대 2) | 완료 |

---

# 구현 내용

## A3. 쇠뇌 폐지

총통이 쇠뇌를 대체했으므로 플레이어 장비는 총통 하나뿐이다.

**삭제**: `AOngseongCrossbowActor`, `UOngseongCrossbowGripComponent`(각 .h/.cpp),
`BP_OngseongCrossbow`, 두 레벨의 배치 인스턴스.

**유지**: `AOngseongBoltProjectileActor`. 이름은 볼트지만 **적 궁병의 화살**로 쓰인다
(`UOngseongArcherCombatComponent::ArrowClass`의 기본값). 지우면 궁병이 화살을 못 쏜다.
주석을 "적 궁병 전용"으로 고쳤다.

테스트 2종에서 쇠뇌 단정문을 제거하고, 총통 그립 하이라이트 검증으로 대체했다.
Core HUD·나레이션에는 쇠뇌 결합이 없었다.

> 레벨에서 액터가 사라지는 방식이 특이하다. 네이티브 클래스를 지우면 배치 인스턴스는
> 로드 단계에서 **조용히 탈락**하고, 레벨을 다시 저장해야 패키지에서 참조가 빠진다.
> 그래서 정리 스크립트는 제거 대상이 없어도 두 레벨을 무조건 다시 저장한다.

## A1. 여분 충차 제거

`BP_OngseongRam_C_1`이 성 **안쪽** (810, 2210, 243)에 배치되어 있었다. 시나리오 목표는
`Ram_ActorPool`에서 획득하는 충차이므로 이 액터는 목표가 아니었는데, 적 진영 파괴 가능
액터라서 아군 총통 사격의 17%를 빨아들이고 있었다. 제거했다.

## A2 + B6. 아군 총통 교전 규칙과 어택 슬롯

### 표적 제한

`AChongtongCannonActor::bEngageEnemyArchersOnly`(기본 true)가 자동 사격 표적을
**적 궁병으로만** 좁힌다. 충차와 검병은 플레이어 몫이다.

판별은 `UOngseongArcherCombatComponent` 보유 여부로 한다. 구체 Actor 클래스를 검사하지
않으므로 `CLAUDE.md` 9절을 지킨다. 플레이어 조준 사격은 방향 기반이라 영향이 없다.

### 어택 슬롯

* 총통 1문당 동시 교전 가능한 적 **2명**(`MaxAttackerSlots`)
* 가득 찬 총통은 건너뛰고 **가장 가까운** 빈 총통을 찾는다
* 전부 차면 **충차 근처로 이동**한다 (`ArcherEscortTarget`, 시나리오가 활성 충차로 설정)
* 사망·퇴각·Pool 반환 시 슬롯 반납, 3초마다 미배정 궁병이 재시도
* 죽었거나 숨겨진 홀더는 조회 시점에 자동으로 정리된다 (풀링 안전망)

**이동 목적지와 사격 표적을 분리했다.** BT의 `TargetActor`는 이동용이고 사격 표적은
Component가 정한다. 이렇게 하지 않으면 호위 중인 궁병이 자기편 충차를 조준한다.

| 상황 | 이동 (`TargetActor`) | 사격 |
|---|---|---|
| 슬롯 확보 | 그 총통 | 그 총통 |
| 슬롯 없음 · 충차 있음 | 충차 | 성문 |
| 슬롯 없음 · 충차 없음 | 성문 | 성문 |

## B1. 충차 회전

`MeshYawOffset`(기본 -90) 신설. 메시가 자기 +Y를 향해 제작되었는데 이동 회전은 +X 기준이라
충차가 옆으로 달렸다. 이동 방향 회전에 이 보정을 더한다. 최종 아트가 반대로 제작되면
에디터에서 부호만 뒤집으면 된다.

## B2. 탄속과 포물선

| 값 | 이전 | 현재 |
|---|---|---|
| 포탄 `ProjectileGravityScale` | 0.15 | **0.7** |
| 포탄 `MaxSpeed` | 8000 | 5000 |
| 플레이어 발사 속도 | 5000 | **2800** |

아군 총통은 직선으로 쏘지 않고 `SuggestProjectileVelocity_CustomArc`로 **포탄 자신의 중력에
맞는 탄도해**를 풀어서 발사한다(`FiringArc = 0.45`). 그래서 느려지고 크게 휘어도 명중률이
유지된다. 탄도해를 못 구하면 사격하지 않고 포신만 표적으로 돌린다.

이를 위해 Shared `AGameplayProjectileActor::GetProjectileGravityScale()`을 추가했다.

## B3. 발사 시 포신 회전

`AimBarrelAtDirection()`이 포신을 탄도 방향으로 돌린 뒤 **포신 축을 따라** 발사한다.
포신의 로컬 전방은 +Y(Muzzle이 BarrelPivot의 +Y에 있음)라서 단순 pitch로는 앙각이 생기지
않는다. 쿼터니언으로 +Y축을 목표 방향에 매핑한다.

"조준한 방향으로 쏜다"가 플레이어와 AI 공통 불변식이 되었다.

## B4. 포구 화염과 포격음

`TryFire()`에 `PlayFeedback(MuzzleEffect, FireSound, 포구 위치)`를 추가했다. 지금까지 이
연출은 플레이어 발사 경로에만 있었다. 에셋은 `NS_MuzzleFlash` + `Fire_Cue`(임시)이고
`SC_OngseongCombat`이 동시 발음을 8음으로 제한한다.

## B5. 검병 Run 애니메이션 고정

**원인**: 적 애니메이션 4종이 전부 `bLoop = false`로 임포트되어 있었다.

```text
AS_MeleeRun: length=0.73s keys=23 rateScale=1.0 loop=False
AS_RangeRun: length=0.57s keys=18 rateScale=1.0 loop=False
AS_EnemyIdle: length=10.0s keys=301 rateScale=1.0 loop=False
```

`UAnimationGraphSchema::SpawnNodeFromAsset`은 애니메이션을 AnimGraph에 드래그할 때
`CopySettingsFromAnimationAsset`을 호출하고, `UAnimGraphNode_SequencePlayer`는 거기서
**에셋의 `bLoop`를 노드의 Loop Animation으로 복사**한다. 그래서 Run 스테이트가 루프 꺼진
상태로 만들어졌고, 0.73초를 한 번 재생한 뒤 마지막 프레임을 붙잡고 있었다. 매끄럽게
순환하도록 만든 달리기 사이클의 마지막 프레임은 첫 프레임과 사실상 같아 보이므로
"첫 자세로 고정"으로 보인다.

**수정**: `AS_MeleeRun`, `AS_RangeRun`, `AS_EnemyIdle`, `AS_AllyIdle`의 `bLoop`를 true로
바꿨다. `AS_Shooting`은 단발 공격이므로 그대로 둔다.

---

# 변경 파일

```text
Plugins/.../Public|Private/Ongseong/OngseongCrossbowActor.*          (삭제)
Plugins/.../Public|Private/Ongseong/OngseongCrossbowGripComponent.*  (삭제)
Plugins/.../Content/Blueprints/BP_OngseongCrossbow.uasset            (삭제)
Plugins/.../Public|Private/Ongseong/ChongtongCannonActor.*
Plugins/.../Public|Private/Ongseong/OngseongEnemyWaveManager.*
Plugins/.../Public|Private/Ongseong/OngseongDefenseScenarioManager.*
Plugins/.../Public/Ongseong/OngseongArcherCombatComponent.h
Plugins/.../Public/Ongseong/OngseongBoltProjectileActor.h
Plugins/.../Public/Ongseong/OngseongRamActor.h
Plugins/.../Private/Ongseong/OngseongRamActor.cpp
Plugins/.../Private/Ongseong/ChongtongProjectileActor.cpp
Plugins/.../Private/Tests/OngseongRangedCombatTests.cpp
Plugins/.../Private/Tests/OngseongInteractionPromptTests.cpp
Source/.../Public|Private/Gameplay/Combat/GameplayProjectileActor.*
Plugins/.../Content/Maps/LV_Ongseong.umap
Plugins/.../Content/Maps/LV_Ongseong_CombatTest.umap
Plugins/.../Content/Asset/Character/Enemy/AS_MeleeRun|AS_RangeRun|AS_EnemyIdle.uasset
Plugins/.../Content/Asset/Character/Instructor/AS_AllyIdle.uasset
Scripts/RemoveOngseongCrossbowAndStrayRam.py                         (신규)
Scripts/DiagnoseEnemyLocomotionAnimation.py                          (신규)
Scripts/FixEnemyLocomotionLooping.py                                 (신규)
docs/OngseongCrossbow/ENEMY_ARCHER_BEHAVIOR_TREE_SETUP.md            (갱신)
```

---

# 테스트 결과

| 검증 | 결과 |
|---|---|
| `SuwonSiegeContestVREditor` Win64 Development | 성공 |
| `SuwonSiegeContestVR` Win64 Development | 성공 |
| `Automation RunTests SuwonSiegeContestVR` | **25/25 성공, 실패 0** |
| `LV_Ongseong` 헤드리스 구동 (약 5분) | 아래 |

## 아군 총통 표적 (105발 전수)

```text
102 firing at BP_EnemyArcher
  3 firing at BP_OngseongArcher
  0 firing at BP_OngseongRam
  0 firing at BP_EnemySword
```

충차·검병 사격 0건. 궁병에 명중한 포탄 84발 — **탄속을 낮추고 곡사로 바꾼 뒤에도 명중한다.**

## 어택 슬롯

```text
BP_AllyChongtong_C_2 attack slot taken by BP_EnemyArcher_C_7 (1/2).
BP_AllyChongtong_C_2 attack slot taken by BP_EnemyArcher_C_6 (2/2).
BP_AllyChongtong_C_0 attack slot taken by BP_EnemyArcher_C_5 (1/2).
BP_AllyChongtong_C_1 attack slot taken by BP_EnemyArcher_C_4 (1/2).
BP_PlayableChongtong_C_1 attack slot taken by BP_EnemyArcher_C_1 (1/2).
BP_AllyChongtong_C_0 attack slot freed by BP_EnemyArcher_C_6 (1/2).
```

어느 총통도 2를 넘지 않고, 궁병이 4문에 고르게 분산되며, 사망 시 슬롯이 반납된다.
슬롯이 남아돌아(총 8칸 vs 궁병 7명) 호위(`escorting=1`) 분기는 이번 구동에서 발생하지 않았다.

## 충차 진행 (설계 의도 확인)

```text
11:41:13  Defense started / Ram advancing at 42 cm/s
11:44:12  Ram struck the gate for 75 damage   <- 접근 179초 = 2분 59초
11:45:11  ... 4.9초 간격으로 13회 타격
```

접근 시간이 목표한 3분과 일치한다.

---

# 주요 결정 사항

* **아군 총통은 이제 충차를 파괴하지 않는다.** 무인 구동은 반드시 실패로 끝난다.
  충차 파괴가 클리어 조건이므로 **플레이어가 충차를 쏘지 않으면 성문이 무너진다.**
  성문 1000 HP ÷ 75 = 14타 → 충차 도착 후 약 68초가 플레이어에게 주어지는 시간이다.
  즉 체험 길이는 준비 3분 + 교전 1분 남짓이다.
* 화살 클래스 이름(`Bolt`)은 쇠뇌 폐지 후에도 유지했다. 이름을 바꾸면 `BP_OngseongBolt`의
  부모 참조가 깨지고 레벨의 화살 Pool을 다시 연결해야 한다. 주석으로 용도를 명시했다.
* 플러그인 이름 `GF_OngseongCrossbow`는 유지한다. 참조 파손 범위가 크고 기능과 무관하다.

---

# 남은 문제

## 1. ABP의 Loop Animation 노드 값 (B5의 마지막 한 걸음)

에셋의 `bLoop`는 고쳤지만, **이미 만들어진 Sequence Player 노드의 `Loop Animation` 값은
에셋을 고쳐도 소급 적용되지 않는다.** Python에서 AnimBlueprint 그래프 노드에 접근할 수
없어 자동화하지 못했다(`function_graphs` 미노출).

에디터에서 다음을 확인해야 한다. 각 1초짜리 작업이다.

| ABP | 확인할 스테이트 | 기대값 |
|---|---|---|
| `ABP_EnemyMelee` | Run | Loop Animation ✔ |
| `ABP_EnemyMelee` | Idle | Loop Animation ✔ |
| `ABP_EnemyArcher` | Run | Loop Animation ✔ |
| `ABP_EnemyArcher` | Idle | Loop Animation ✔ |

체크가 이미 되어 있다면 원인은 다른 곳이며, `Scripts/DiagnoseEnemyLocomotionAnimation.py`가
남긴 다른 후보(Sequence Evaluator 사용 여부)를 확인해야 한다.

## 2. 육안 확인이 필요한 항목

헤드리스(`-nullrhi`)로는 검증할 수 없다.

* 충차가 진행 방향을 향해 똑바로 달리는지 (`MeshYawOffset` 부호)
* 아군 총통 포신이 표적을 향해 도는지
* 발사 시 포구 화염과 포격음
* 포탄의 포물선 궤적 체감
* 인터랙션 발광(파란 대포알, 호박색 그립)

## 3. 호위 분기 미검증

총통 8슬롯 > 궁병 7명이라 평시에는 전원이 슬롯을 얻는다. 충차 호위 경로는 총통이
파괴되었을 때만 발생하며 이번 구동에서는 재현되지 않았다. BT의 `Move To` Acceptable
Radius 3150이 호위 상황에는 지나치게 멀다는 문제도 함께 남아 있다
(`ENEMY_ARCHER_BEHAVIOR_TREE_SETUP.md` §4).

## 4. 기존 잔여 항목

PIE/HMD 육안 확인, Android 성능 실측, 최종 메시·애니메이션·SFX·나레이션 녹음,
PlayerPhone 기능 미정, NavMesh 북쪽 끝 경계.
