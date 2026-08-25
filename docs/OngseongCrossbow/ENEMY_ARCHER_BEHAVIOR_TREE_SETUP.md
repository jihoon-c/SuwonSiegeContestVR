# 적 궁병 Behavior Tree 설정

**2026-08-24 갱신**: 최초 작성 시점에는 "에디터에서 만들어야 하는 것" 목록이었으나,
BB/BT/AIController 에셋은 모두 생성되어 있다. 아래는 **현재 구성 기준 문서**이며,
아군 총통 어택 슬롯 도입에 따른 변경 사항을 §4에 반영했다.

## 0. 에셋 위치

| 에셋 | 경로 | 상태 |
|---|---|---|
| Blackboard | `/GF_OngseongCrossbow/AI/BB_EnemyArcher` | 존재 |
| Behavior Tree | `/GF_OngseongCrossbow/AI/BT_EnemyArcher` | 존재 |
| AIController | `/GF_OngseongCrossbow/AI/BPC_EnemyArcherAIController` | 존재 |

궁병 BT는 **이 체험 전용 진행 스크립트**이므로 Game Feature 플러그인 안에 둔다.
공용 `Content/Gameplay/AI/Behavior/`는 비어 있고, 여러 체험이 공유하는 BT가 생길 때까지
그대로 둔다 (`CLAUDE.md` 5절).

## 1. 에디터 재시작이 필요한 경우

`UCLASS` 추가·삭제가 포함된 변경은 에디터를 닫은 상태에서
`SuwonSiegeContestVREditor Win64 Development`를 빌드한 뒤 다시 연다.
Live Coding으로 새 reflected class를 주입하지 않는다.

## 2. Blackboard 키

| Key | Type | Base Class | 의미 |
|---|---|---|---|
| `TargetActor` | Object | Actor | **이동 목적지.** 사격 표적이 아니다 (§4 참조) |
| `IsAttacking` | Bool | - | 공격 애니메이션 상태 |

키 이름은 `ACombatAIController`의 기본 키 이름과 정확히 일치해야 한다.

## 3. Behavior Tree 구조

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

`Fire Ongseong Arrow`는 네이티브 `UBTTask_OngseongFireArrow`다. **Blackboard를 읽지 않고**
Pawn의 `UOngseongArcherCombatComponent::TryFireArrow()`를 호출한다. 즉 이동은 Blackboard가,
사격은 Component가 결정한다. 화살 발사는 이 BT 태스크가 유일한 경로이므로,
**BT가 없으면 궁병은 화살을 쏘지 않는다.**

`3150`은 기본 `ArcherRange` 3500의 90%다. Blueprint에서 사거리를 바꾸면 이 값도
같은 비율로 조정한다.

`Observe Blackboard Value: true` 덕분에 `TargetActor`가 바뀌면 이동이 즉시 중단되고
새 목적지로 다시 경로를 잡는다. 어택 슬롯 재배정이 이 동작에 의존한다.

## 4. 아군 총통 어택 슬롯 (2026-08-24 추가)

### 규칙

* 아군 총통 1문에 동시에 붙을 수 있는 적은 **최대 2명**(`AChongtongCannonActor::MaxAttackerSlots`)
* 슬롯이 가득 찬 총통은 **무시**하고 다음 총통을 찾는다
* 모든 총통이 가득 차면 **충차 근처로 이동**한다 (`ArcherEscortTarget`)
* 궁병이 죽거나 퇴각하거나 Pool로 반환되면 슬롯이 즉시 반납된다
* 슬롯을 못 얻은 궁병은 `ArcherSlotRetryInterval`(기본 3초)마다 재시도한다

### 이동 목적지와 사격 표적의 분리

`AOngseongEnemyWaveManager::ApplyArcherEngagement()`가 궁병 1명마다 다음을 정한다.

| 상황 | `TargetActor` (BT 이동) | 사격 표적 (Component) |
|---|---|---|
| 슬롯 확보 | 그 총통 | 그 총통 |
| 슬롯 없음 · 충차 있음 | 충차 | 성문 (fallback) |
| 슬롯 없음 · 충차 없음 | 성문 | 성문 |

**궁병은 자기 진영 충차를 쏘지 않는다.** 충차는 이동 목적지로만 쓰이고, 사격 표적은
`ConfigureCombat(Cannon, ObjectiveTarget, Pool)`의 Primary/Fallback으로 결정된다.

가장 가까운 총통부터 시도하므로 궁병이 지나가는 순서대로 자연스럽게 분산된다.

### 알려진 튜닝 항목

* 궁병 슬롯 7명 vs 총통 4문 × 2슬롯 = 8칸이므로 **평시에는 전원이 슬롯을 얻는다.**
  호위(충차) 분기는 총통이 파괴되었을 때 주로 발생한다.
* `Move To`의 `Acceptable Radius: 3150`은 **총통 교전 거리** 기준이다. 충차 호위로 갈 때는
  충차에서 3150cm 떨어진 곳에 멈추므로 "근처"라기엔 멀다. 호위 연출을 다듬으려면
  호위 전용 분기(작은 Acceptable Radius를 가진 두 번째 `Move To`)를 BT에 추가해야 한다.
  현재는 위 이유로 발생 빈도가 낮아 보류했다.

## 5. AIController Blueprint

1. `/GF_OngseongCrossbow/AI/BPC_EnemyArcherAIController`의 부모는 `CombatAIController`
2. Class Defaults의 `Behavior Tree Asset`은 `BT_EnemyArcher`
3. `BP_EnemyArcher`의 `AI Controller Class`가 위 Blueprint
4. `Auto Possess AI`는 `Placed in World or Spawned`

검병 `BP_EnemySword`는 Behavior Tree를 지정하지 않는다. 기본 `CombatAIController`가
Wave Manager의 직접 `Move To` 요청만 수행한다.

## 6. 애니메이션 연결 확인

두 AnimBP의 `Event Blueprint Update Animation`은 이동 속도를 갱신한다.

```text
Speed = VectorLengthXY(TryGetPawnOwner → GetVelocity)
```

각 캐릭터 Blueprint(`BP_EnemyArcher`, `BP_EnemySword`)의 `Event Tick`은 다음처럼
공격 상태를 대응하는 AnimBP로 전달한다.

```text
Get Mesh → Get Anim Instance → Cast 대응 AnimBP
Self → IsAttacking → Set bIsAttacking
```

- `ABP_EnemyArcher`의 Attack 전이는 `bIsAttacking == true`로 진입한다.
- Attack에서 Idle/Run으로 나가는 전이는 `bIsAttacking == false`를 사용한다.
- `ABP_EnemyMelee`도 동일한 변수를 사용한다.

### Run 애니메이션이 첫 자세에서 멈추던 문제 (2026-08-25 해결)

**원인**: 적 애니메이션이 전부 `bLoop = false`로 임포트되어 있었다.
`UAnimationGraphSchema::SpawnNodeFromAsset`은 애니메이션을 AnimGraph에 드래그할 때
`UAnimGraphNode_SequencePlayer::CopySettingsFromAnimationAsset`으로 **에셋의 `bLoop`를
노드의 Loop Animation에 복사**한다. 그래서 Run 스테이트가 루프 꺼진 채로 만들어졌고,
0.73초짜리 사이클을 한 번 재생한 뒤 마지막 프레임을 붙잡았다. 매끄럽게 순환하는 달리기
사이클은 마지막 프레임이 첫 프레임과 거의 같아서 "첫 자세 고정"으로 보인다.

**수정**: `AS_MeleeRun` `AS_RangeRun` `AS_EnemyIdle` `AS_AllyIdle`의 `bLoop`를 true로 바꿨다
(`Scripts/FixEnemyLocomotionLooping.py`). `AS_Shooting`은 단발 공격이라 그대로 둔다.

> **남은 수동 작업**: 에셋을 고쳐도 **이미 만들어진 노드의 Loop Animation은 바뀌지 않는다.**
> `ABP_EnemyMelee`와 `ABP_EnemyArcher`를 열어 Run·Idle 스테이트의 Sequence Player에서
> Loop Animation이 체크되어 있는지 확인한다. Python에서 AnimBP 그래프 노드에 접근할 수
> 없어 자동화하지 못했다.

## 7. 레벨 확인

- 궁병 Spawn 지점부터 목표까지 `NavMeshBoundsVolume`이 이어져 있어야 한다.
- 에디터에서 `P` 키를 눌러 녹색 NavMesh를 확인한다.
- PIE에서 궁병의 Blackboard `TargetActor`가 총통 또는 충차로 설정되는지 확인한다.
- **아군 총통은 2026-08-24부터 적 궁병만 표적으로 삼는다**
  (`AChongtongCannonActor::bEngageEnemyArchersOnly`, 기본 true).
  충차와 검병은 플레이어 몫이다. 총통과 궁병 사이에 Visibility를 차단하는 구조물이 있으면
  그 궁병은 표적에서 제외된다.
