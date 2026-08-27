# 옹성 총통·적 병사·궁병 수정 사항

## 코드 반영

* `AChongtongCannonActor`의 포구 파티클은 `Muzzle Effect Scale` 기본값 `2.0`으로 재생된다.
* 총통의 `StatusText` 및 그에 딸린 상태등 컴포넌트를 네이티브 액터에서 제거했다.
* `AOngseongEnemyWaveManager`의 **Maximum Spawned Enemy Soldiers**는 레벨 Details에서 조절할 수 있다.
  검병/궁병별 실제 인원은 `Swordsman Slots`, `Archer Slots`가 정하며, 두 슬롯 합계와 최대 수 중
  작은 값까지만 스폰된다.
* 궁병은 `IsAttacking` AI/AnimBP 상태를 더 이상 설정하지 않는다. `Attack Animation`에 지정된
  `UAnimSequenceBase`를 AnimBP Slot으로 끝까지 재생한 뒤 화살을 발사한다. 기본값은 `AS_Shooting`이다.
* 궁병은 예약된 총통과 플레이어 중 살아 있고 가까운 쪽을 표적으로 삼는다. `Archer Player Target`을
  지정하지 않으면 Player 0 Pawn을 자동으로 찾는다.
* 총통 한 대당 궁병 공격 포지션 예약 제한을 `Archer Attack Slots Per Cannon = 3`으로 바꿨다.

## 레벨/블루프린트에서 할 일

1. `LV_Ongseong`의 `Ongseong_WaveManager`를 선택하고 원하는 **Maximum Spawned Enemy Soldiers**,
   `Swordsman Slots`, `Archer Slots`를 함께 설정한다. 예: 최대 18명을 원하면 슬롯 합계도 18 이상이어야 한다.
2. 같은 액터에서 필요하면 **Archer Player Target**에 플레이어 Pawn(또는 플레이어를 대표하는 Actor)을
   연결한다. 비워 두면 런타임에 Player 0을 사용한다.
3. `BP_EnemyArcher`의 `OngseongArcherCombat` 컴포넌트 Defaults에서 **Attack Animation**에 원하는
   공격 시퀀스를 배정한다. `Attack Animation Slot` 이름은 아래 Slot 노드와 일치해야 한다.
4. `ABP_EnemyArcher`에서 `bIsAttacking` / `IsAttacking`으로 Attack 상태에 진입하거나 빠져나오는
   전이와, 캐릭터 Blueprint가 그 변수를 갱신하는 로직을 삭제한다. 대신 AnimGraph의 locomotion 출력과
   최종 Output Pose 사이에 `Slot` 노드를 하나 추가하고 Slot Name을 `DefaultSlot`으로 둔다.
5. 총통마다 기존 `Ongseong.ArcherAttackPosition` 태그 TargetPoint가 2개라면 **1개씩 추가**해 총 3개로
   만든다. 새 포인트도 성벽 안쪽 NavMesh 위, 해당 총통 사거리 안에 배치하고 같은 Actor Tag를 넣는다.
   현재 선택 규칙은 포인트에서 가장 가까운 생존 총통을 자동으로 연결하므로 별도 Actor 참조는 필요 없다.
6. PIE에서 궁병 공격 애니메이션이 완전히 끝난 직후에 화살이 생성되는지, 그리고 플레이어가 총통보다
   가까울 때 플레이어를 향해 발사하는지 확인한다.
