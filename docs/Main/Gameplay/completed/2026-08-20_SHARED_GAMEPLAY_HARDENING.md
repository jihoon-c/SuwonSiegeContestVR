# Shared Gameplay 안정화 완료

**완료일**: 2026-08-20

## 적용 사항

- 풀 반환 시 Enemy 공격 타이머, 표적, 단순 이동, CharacterMovement와 고정밀 AI를 정지한다.
- 풀 재획득 시 체력과 사망 상태를 초기화하고 LOD 관리 시스템을 다시 활성화한다.
- Actor Pool이 활성/대기 Actor를 함께 세어 `InitialPoolSize` 상한을 지키며 유효하지 않은 참조를 정리한다.
- 원거리 AI LOD가 Mesh visibility뿐 아니라 관련 Component Tick과 CharacterMovement도 비활성화한다.
- `SetCurrentHealth(0)`도 정상적인 사망 전이와 `OnDeath` 이벤트를 발생시킨다.
- 공용 Projectile이 Poolable 계약을 구현하고, Pool 소유 Projectile은 충돌하거나 수명이 끝날 때 Destroy 대신 반환된다.
- 총통이 선택적으로 Actor Pool에서 Projectile을 획득할 수 있으며 미설정 시 기존 Spawn 방식으로 동작한다.
- `EnemySimpleMovementComponent`의 기본 갱신 주기와 Clamp metadata 불일치를 바로잡았다.

## 자동화 검증

- UBT: 타깃 Include Order를 UE 5.8로 갱신한 뒤 `SuwonSiegeContestVREditor Win64 Development -NoHotReload` 성공 (24 actions)
- Automation: `SuwonSiegeContestVR.Gameplay` 4/4 성공, Warning 0, Error 0
  - Faction 적대 판정
  - Health 사망/재사용 초기화
  - Actor Pool 용량 불변식
  - Projectile Poolable 계약
- Unreal MCP: `LV_Ongseong` Simulate PIE 정상 시작·종료, Map Check 및 PIE 구간 Warning/Error 0

## 후속 에디터 연결 결과

`LV_Ongseong`의 네이티브 총통 `GateTarget`, 적·투사체 `PooledActorClass`, 성문 Health/Faction과
`AOngseongEnemyWaveManager`를 연결했다. 공용 `/Game/Gameplay/Characters/BP_EnemySoldier`는 Manny를
사용하는 기능 검증용 임시 자산이다. MCP Simulate PIE에서 적 최대 6개 활성, 목표 이동·공격,
성문 피해와 총통 투사체의 Pool 재사용을 확인했다.

최종 적 메시/애니메이션, 총통/포탄 시각·VFX·SFX, Muzzle/충돌 튜닝, 근거리 BT/StateTree,
체험 완료 조건과 Android 실기기 프로파일링은 콘텐츠 담당 후속 작업이다.

Android SDK(r27c)와 OpenXR Runtime은 로컬 환경에서 유효하지 않아 Android 실기기 및 VR 검증은 별도 환경 설정 후 수행한다.
