# 적 궁병 Behavior Tree 설정

Unreal MCP의 현재 Asset 도구는 `BehaviorTree`와 `BlackboardData` 에셋 생성을 지원하지 않는다.
아래 두 에셋만 에디터에서 만들면 C++와 Blueprint 쪽 연결은 준비된 상태로 동작한다.

## 1. 에디터 재시작

이번 변경은 `UCLASS` 추가와 기존 AI 클래스 삭제를 포함한다. 에디터를 닫은 상태에서
`SuwonSiegeContestVREditor Win64 Development`를 빌드한 뒤 다시 연다. Live Coding으로
새 reflected class를 주입하지 않는다.

## 2. Blackboard 생성

`/GF_OngseongCrossbow/AI/BB_EnemyArcher`를 생성하고 다음 키를 추가한다.

| Key | Type | Base Class |
|---|---|---|
| `TargetActor` | Object | Actor |
| `IsAttacking` | Bool | - |

키 이름은 `ACombatAIController`의 기본 키 이름과 정확히 일치해야 한다.

## 3. Behavior Tree 생성

`/GF_OngseongCrossbow/AI/BT_EnemyArcher`를 생성하고 Blackboard를
`BB_EnemyArcher`로 지정한다.

```text
Root
└─ Sequence  [Blackboard: TargetActor Is Set]
   ├─ Move To
   │  ├─ Blackboard Key: TargetActor
   │  ├─ Acceptable Radius: 3150
   │  ├─ Stop on Overlap: false
   │  └─ Observe Blackboard Value: true
   ├─ Fire Ongseong Arrow
   └─ Wait
      ├─ Wait Time: 2.75
      └─ Random Deviation: 0.25
```

`Fire Ongseong Arrow`는 이번 변경에서 추가된 네이티브
`UBTTask_OngseongFireArrow` 노드다. 발사 성공 시 궁병 캐릭터의 공격 상태를 켜고,
`AttackAnimationDuration` 후 자동으로 끈다.

`3150`은 기본 `ArcherRange` 3500의 90%다. Blueprint에서 사거리를 바꾸면 이 값도
같은 비율로 조정한다.

## 4. AIController Blueprint 생성

1. `/GF_OngseongCrossbow/AI/BPC_EnemyArcherAIController` Blueprint를 만든다.
2. 부모 클래스를 `CombatAIController`로 지정한다.
3. Class Defaults의 `Behavior Tree Asset`에 `BT_EnemyArcher`를 지정한다.
4. `BP_EnemyArcher`의 `AI Controller Class`를 위 Blueprint로 지정한다.
5. `Auto Possess AI`는 `Placed in World or Spawned`로 둔다.

검병 `BP_EnemySword`는 Behavior Tree를 지정하지 않는다. 기본 `CombatAIController`가
Wave Manager의 직접 `Move To` 요청만 수행한다.

## 5. 애니메이션 연결 확인

두 AnimBP의 `Event Blueprint Update Animation`은 이동 속도를 갱신한다.

```text
Speed = VectorLengthXY(TryGetPawnOwner → GetVelocity)
```

각 캐릭터 Blueprint(`BP_EnemyArcher`, `BP_EnemySword`)의 `Event Tick`은 다음처럼
공격 상태를 대응하는 AnimBP로 전달한다. 이 그래프는 Unreal MCP로 구성되어 있다.

```text
Get Mesh → Get Anim Instance → Cast 대응 AnimBP
Self → IsAttacking → Set bIsAttacking
```

- `ABP_EnemyArcher`의 Attack 전이는 `bIsAttacking == true`로 진입한다.
- Attack에서 Idle/Run으로 나가는 전이는 `bIsAttacking == false`를 사용한다.
- `ABP_EnemyMelee`도 동일한 변수를 사용하므로 향후 검병 공격을 추가해도 그래프를
  다시 만들 필요가 없다.

## 6. 레벨 확인

- 궁병 Spawn 지점부터 목표까지 `NavMeshBoundsVolume`이 이어져 있어야 한다.
- 에디터에서 `P` 키를 눌러 녹색 NavMesh를 확인한다.
- PIE에서 궁병의 Blackboard `TargetActor`가 아군 총통 또는 보조 목표로 설정되는지 확인한다.
- 총통과 궁병 사이에 Visibility를 차단하는 구조물이 있으면 아군 총통은 해당 궁병을
  표적으로 선택하지 않는다.
