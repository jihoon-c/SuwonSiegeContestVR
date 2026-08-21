# 공심돈 Enemy Group

## 구조

```text
Gongsimdon_EnemyGroup
  ├─ ApproachSpline
  ├─ RetreatSpline
  ├─ ObservationAnchor / OBS_ENEMY_GROUP
  ├─ CombatInteraction / COMBAT_RETREATING
  └─ AEnemySoldierActor × 7 (런타임 Spawn)
       ├─ UHealthComponent
       └─ UFactionComponent = Enemy
```

병사는 Shared Gameplay에 있고, 대형과 Scenario 연출은 `GF_Gongsimdon`에 있다.

## 상태

```text
Hidden
  → CUE_REVEAL_ENEMY
Approaching
  → Spline 끝
Holding
  → CUE_RETREAT_ENEMY
Retreating
  → 유효 피격
Escaped
```

퇴각 Spline 끝까지 피격되지 않으면 마지막 위치에서 대기해 Interaction이 막히거나 사격 기회가 사라지지 않는다.

## 주요 변수

| 변수 | 기본값 | 역할 |
|---|---:|---|
| `EnemyCount` | 7 | Spawn 병사 수, 6~8 제한 |
| `FormationColumns` | 3 | 대형 열 수 |
| `LateralSpacing` | 140 | 좌우 간격(cm) |
| `RowSpacing` | 180 | 앞뒤 간격(cm) |
| `ApproachSpeed` | 140 | 접근 속도(cm/s) |
| `RetreatSpeed` | 260 | 퇴각 속도(cm/s) |
| `RequiredCombatHits` | 1 | Combat 완료 필요 명중 수 |

Spline은 Level의 `Gongsimdon_EnemyGroup` Actor를 선택해 직접 편집한다.

## 시각 자산

현재 `AEnemySoldierActor`는 적의 존재 확인을 위한 Manny 임시 Mesh를 사용한다. Niagara Gallery Anim Blueprint는 커맨드렛 초기화 오류를 유발해 기본 연결에서 제외했다. 실제 제작 시 `AEnemySoldierActor` 파생 Blueprint를 만들고 다음을 설정한다.

- 조선시대 적군 Skeletal Mesh
- 보행/대기/퇴각 Animation Blueprint
- 피격 Reaction
- Group Actor의 `EnemyClass`를 해당 Blueprint로 교체

## 사격 연결

무기 또는 투사체의 Hit에서 맞은 Soldier에 표준 Unreal `ApplyDamage`를 호출한다. Health 변화는 Group이 수신하며 `COMBAT_RETREATING`을 완료한다. 구체적인 Enemy Class 검사는 하지 않는다.
