# Shared Gameplay — 현재 상태

**갱신일**: 2026-08-20
**구현 커밋**: `bb37e29` (`feat: add shared combat AI and chongtong defense`)

## 완료된 C++ 기반

| 영역 | 상태 | 제공 기능 |
|---|---|---|
| Character | Implemented | `ACombatCharacter`, `AEnemyCombatCharacter`, `AAllyCombatCharacter` |
| Faction / Health / Damage | Implemented | 진영 적대 판정, 체력·사망 이벤트, Damage Spec, 수신 Interface, 적용 Utility |
| Projectile | Implemented | 충돌·수명·진영 판정을 수행하는 `AGameplayProjectileActor` |
| Enemy AI | Implemented | 원거리 단순 이동, 근거리 BT/StateTree 전환 훅, `Advance`/`Assault`/`Retreat` 상태 |
| AI LOD | Implemented | 원거리 메시·CharacterMovement·고정밀 AI 비활성화, 근거리 렌더·정밀 AI 전환, 거리 히스테리시스 |
| Pooling | Implemented | 고정 용량 사전 생성, 적의 공격/AI/이동 초기화, Enemy·Projectile 재사용 계약 |
| Combat AI 지원 | Implemented | 공격자 위협 추적, 적대 표적 검색, 범위 내 주기 공격 |

## 구현된 시나리오 연결 API

| 시나리오 | 연결 방식 |
|---|---|
| 웅성/총통 적 | `SetObjectiveTarget(성문/총통)`으로 전진 후 도착 시 공격 |
| 공심돈 적 | 발각 시 `SetRetreatTargetLocation(탈출점)`으로 도주 |
| 신기전 적 | 고정 목표를 `SetObjectiveTarget`으로 지정해 일방향 전진 |
| 총통 | 공격자 → 성문 인접 적 → 무작위 적 순으로 선택해 투사체 발사 |

## 미완료 작업

1. 공용 `/Game/Gameplay/Characters/BP_EnemySoldier`의 Manny 임시 메시·애니메이션을 최종 적 병사 자산으로 교체한다.
2. `BP_ChongtongCannon`, `BP_ChongtongProjectile`을 만들어 포신 Muzzle, 메시, VFX, SFX를 연결한다. 현재 네이티브 총통은 발사하지만 배치 지형에 먼저 충돌하므로 Muzzle 위치와 충돌 채널을 조정해야 한다.
3. 근거리 Enemy용 BT 또는 StateTree Asset을 작성하고 `OnHighDetailAIChanged`에 연결한다.
4. 웨이브 종료·성문 파괴 시 성공/실패를 `ExperienceSubsystem` 및 UI에 보고하는 Feature 진행 로직을 완성한다.
5. Android 실기기에서 풀 크기, LOD 거리, 동시 투사체·적 수를 프로파일링하고 조정한다.

## 최신 검증

- 2026-08-19: Editor 종료 후 `SuwonSiegeContestVREditor Win64 Development -NoHotReload` 빌드에 성공했다.
- 빌드 과정에서 UE 5.8의 `TMap<TObjectPtr<AActor>, ...>` 키 추출 타입 불일치를 수정했다.
- 2026-08-20: 8000번 MCP로 `LV_Ongseong`의 `ChongtongCannonActor_0`, `ActorPool_0` 배치를 확인했다.
- 2026-08-20: 풀 생명주기·용량 상한·원거리 Component Tick·Health reset·Projectile 재사용 계약을 보강하고 자동화 테스트를 추가했다.
- 2026-08-20: 타깃 Include Order를 UE 5.8로 갱신하고 UBT 전체 에디터 타깃 빌드(24 actions)에 성공했다.
- 2026-08-20: `SuwonSiegeContestVR.Gameplay` Automation Test 4/4가 성공했으며 Warning/Error는 0건이다.
- 2026-08-20: 새 DLL을 로드한 뒤 MCP로 `LV_Ongseong` Simulate PIE를 시작·종료했다. Map Check와 PIE 구간 모두 Error/Warning 0건이다.
- 2026-08-20: `AOngseongEnemyWaveManager`와 구성 Automation Test를 추가하고 UBT 증분 빌드에 성공했다.
- 2026-08-20: 전체 `SuwonSiegeContestVR` Automation Test 10/10이 성공했으며 Warning/Error는 0건이다.
- 2026-08-20: 공용 `BP_EnemySoldier`를 생성하고 `LV_Ongseong`의 적 Pool(8), 최대 활성 적(6), 성문 Health/Faction, 총통 GateTarget, Projectile Pool(16)을 연결했다.
- 2026-08-20: MCP Simulate PIE에서 적 6개 활성, `Advance` → `Assault`, 성문 피해, 총통 투사체 발사 후 Pool 반환을 확인했다. 저장·재로딩 뒤 Map Check는 Error/Warning 0건이다.
- 환경 검사에서 Win64 SDK는 유효했지만 Android SDK(r27c)와 OpenXR Runtime은 준비되지 않은 상태다.

## 에디터 MCP 상태

`http://127.0.0.1:8000/mcp`에서 Editor, Log, Actor, Asset, Blueprint, Object, Scene 등
에디터 Toolset이 노출된다. 2026-08-20 확인 시 `LV_Ongseong`이 열려 있었고 PIE는 실행 중이 아니었다.
단, C++ UBT 빌드와 Automation Test 실행 Tool은 노출되지 않아 외부 UBT/Editor-Cmd 검증이 필요하다.
