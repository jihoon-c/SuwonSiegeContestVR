# 작업

비VR 테스트 모드에서 아군 총통이 적을 한 명도 공격하지 못하던 문제를 원인까지 추적해 해결했다.
증상은 5분 구동 내내 `has no target to fire at`이었고, 원인은 코드 1건과 레벨 구성 4건이었다.

---

# 원인 분석

헤드리스 `-game` 구동 + 에디터 Python 실측으로 하나씩 좁혔다.

## 1. 총통이 자기 발판을 보고 있었다 (코드)

`UCombatTargetingComponent::HasLineOfSightTo`는 `GetActorEyesViewPoint`에서 트레이스를 시작한다.
`AActor` 기본 구현은 **액터 원점**을 돌려주는데, 총통 원점은 z=600으로 총통이 올라선 성벽
(`Battlement_GateDefense`) 표면과 같은 높이다. 실측 로그:

```text
Sight to BP_OngseongRam_C_1 blocked by StaticMeshActor_22 at V(X=3210.77, Y=3494.36, Z=595.70)
```

총통 원점 (3240, 3510, 600)에서 **35 cm 떨어진 자기 발판**에 트레이스가 막혔다. 사거리 안의
적 11~12명이 전부 "보이지 않음"으로 걸러졌다.

## 2. 레벨에 걸을 수 있는 바닥이 없었다 (레벨 에셋)

`ground` 액터(`SM_Ground`, 스케일 70×90)는 `BlockAll`에 콜리전이 켜져 있었지만,
박스 콜리전 2개의 **높이가 0**이었다.

```text
local box centre (0,0,0) size (100 x 100 x 0)  ->  world size (7000 x 9000 x 0)
downward probe (150, 4000) : NO FLOOR
```

두께 0 박스는 충돌하지 않는다. 스폰된 적 15명, 레벨에 배치된 샘플 적, 아군 병사가 전부 추락해
사라졌다. 총통과 충차는 중력을 쓰지 않아 남아 있었기 때문에 증상이 "적만 없음"으로 보였다.

## 3. 적 스포너가 월드 원점에 고정돼 있었다 (코드 + 레벨)

`AOngseongEnemyWaveManager`에 **루트 컴포넌트가 없어** 에디터에서 액터를 옮길 수 없었다.
`set_actor_location`이 조용히 실패하고 스폰 위치가 (0,0,0)에 고정된다. 그 지점은 통로 바닥
(y 80~9080) 바깥이라 적이 허공에서 생성됐다. `AOngseongDefenseScenarioManager`도 같은 상태였다.

## 4. 총통 4문이 성벽 바깥을 향하고 있었다 (레벨)

```text
BP_AllyChongtong  액터 (3240, 3510, 600)  포구 (3850, 3660, 700)
BP_AllyChongtong2 액터 (-2890, 3210, 600) 포구 (-3500, 3060, 700)
```

포구가 성벽선(x ≈ ±3300) **바깥**에 있었다. 통로 안쪽 적을 조준하면 포탄이 즉시 성벽에 맞았다.
실측: 43발 중 34발이 `StaticMeshActor_22/149/152`(성벽)에 착탄, 일부는 아군 병사에 착탄.

## 5. 적 스폰 지점이 NavMesh 밖이었다 (레벨)

NavMeshBoundsVolume이 z −190~10으로 바닥(z=10) 위 공간을 못 덮어 NavMesh가 생성되지 않았다.
볼륨을 고친 뒤에도 y=8200 스폰은 NavMesh 경계(y ≈ 7961) 밖이라 적이 전진하지 못했다.

---

# 구현 내용

## 코드 (GF_OngseongCrossbow)

* `AChongtongCannonActor::GetActorEyesViewPoint` 오버라이드 — 시야 판정을 **포신**에서 시작한다.
  포탄이 실제로 떠나는 지점이므로 "맞출 수 있는가"의 정직한 기준이다.
* `AOngseongEnemyWaveManager` / `AOngseongDefenseScenarioManager`에 `USceneComponent` 루트 추가 —
  스포너와 시나리오 매니저를 에디터에서 배치할 수 있다.
* `LogOngseong` 진단 추가: 표적 없음 시 사거리 내 적 수와 **시야를 막은 액터·좌표**,
  스폰 시 NavMesh 투영 결과, 포탄 착탄 대상과 남은 체력, 적 사망 시 성문까지 거리.
* 모듈에 `NavigationSystem` 의존성 추가.

## 에셋

* `SM_Ground`의 `collision_trace_flag`를 **`Use Complex As Simple`** 로 변경.
  평평한 정적 바닥에 맞는 설정이며, 이제 전 구간에서 바닥이 z=10에 존재한다.
  **이 에셋은 본편 `LV_Ongseong`도 사용하므로 두 레벨 모두에 적용된다.**

## 테스트 레벨 (`LV_Ongseong_CombatTest`만)

| 대상 | 변경 |
|---|---|
| `Ongseong_WaveManager` | (0,0,0) → **(150, 7800, 98)** — 통로 북쪽, NavMesh 안, 성문 방향(yaw −90) |
| `Ongseong_RamSpawnPoint` | (0,0,0) → **(150, 8350, 243)**, 시나리오 `RamSpawnPoint`에 연결 |
| `Ongseong_RetreatPoint` | (150, −3900) → **(150, 8900, 98)** — 적이 들어온 방향 |
| 총통 4문 | **yaw +180°** — 포구가 통로 안쪽을 향한다 |
| NavMeshBoundsVolume | 원점 (0,3950,−90)·범위 (2000,4000,100) → **(250,4580,260)·(3600,4600,400)** 후 Navigation 재빌드 |

스크립트: `Scripts/FixOngseongCombatTestLayout.py` (멱등, 이동 실패 시 경고),
`Scripts/FixOngseongGroundCollision.py`, 조사용 `InspectOngseongLineOfSight/Ground/GroundBoxes/Layout/Actors.py`.

---

# 테스트 결과

## Automation

`SuwonSiegeContestVR` 전체 **24/24 통과** (exit code 0).

## 비VR 테스트 레벨 4분 구동 (`-game -nullrhi`)

| 항목 | 수정 전 | 수정 후 |
|---|---|---|
| 총통 발사 | 0 | 15 |
| 적 처치 | 0 | **15** |
| 적 사망 지점(성문까지 거리) | — | 최소 1,827 / 평균 6,607 / 최대 7,719 cm |
| 포탄 착탄 대상 | 43발 중 34발 성벽 | 적·충차 위주 |
| 충차 | 성문 13회 타격 → 실패 | **파괴됨 → `Defense succeeded`** |
| Pool 고갈 경고 | 0 | 0 |

적이 통로를 따라 성문 1,827 cm 앞까지 전진하고, 총통이 이를 요격하며, 최종적으로 충차가 파괴되어
클리어 조건이 성립하는 전체 루프를 확인했다.

---

# 남은 문제

## 본편 `LV_Ongseong`에는 아직 적용하지 않았다

같은 레벨 구성 문제(스포너 원점 고정, 총통 방향, NavMesh 볼륨 z 범위, 충차 스폰 지점 미설정)가
본편 레벨에도 그대로 있다. 코드 수정과 `SM_Ground` 콜리전 수정은 이미 양쪽에 적용되지만,
**액터 배치 변경은 실제 체험의 전투 구도를 바꾸므로 레벨 담당자 확인 후 적용해야 한다.**
적용은 `Scripts/FixOngseongCombatTestLayout.py`의 `LEVEL` 상수만 바꾸면 된다.

## 그 밖

* 사람이 PIE로 열어 자유 카메라 조작감과 전투 연출을 육안 확인
* 충차 접근 시간: 스폰 (150, 8350)에서 성문까지 8,250 cm를 35 cm/s로 이동 → 약 4분.
  체험 길이에 맞춰 `MoveSpeed` 또는 스폰 거리를 조정해야 한다.
* NavMesh가 y ≈ 7961까지만 생성된다. 통로 북쪽 끝까지 덮으려면 볼륨과 바닥 경계를 다시 봐야 한다.
* `SC_OngseongCombat` Sound Concurrency 에셋 생성 및 연결
* Android 실기기 성능 실측
