# Shared Gameplay — 현재 상태

**갱신일**: 2026-08-19  
**구현 커밋**: `bb37e29` (`feat: add shared combat AI and chongtong defense`)

## 완료된 C++ 기반

| 영역 | 상태 | 제공 기능 |
|---|---|---|
| Character | Implemented | `ACombatCharacter`, `AEnemyCombatCharacter`, `AAllyCombatCharacter` |
| Faction / Health / Damage | Implemented | 진영 적대 판정, 체력·사망 이벤트, Damage Spec, 수신 Interface, 적용 Utility |
| Projectile | Implemented | 충돌·수명·진영 판정을 수행하는 `AGameplayProjectileActor` |
| Enemy AI | Implemented | 원거리 단순 이동, 근거리 BT/StateTree 전환 훅, `Advance`/`Assault`/`Retreat` 상태 |
| AI LOD | Implemented | 원거리 메시 숨김·저빈도 처리, 근거리 렌더·고정밀 AI 전환, 거리 히스테리시스 |
| Pooling | Implemented | 사전 생성 Actor Pool, Poolable 재사용 계약 |
| Combat AI 지원 | Implemented | 공격자 위협 추적, 적대 표적 검색, 범위 내 주기 공격 |

## 구현된 시나리오 연결 API

| 시나리오 | 연결 방식 |
|---|---|
| 웅성/총통 적 | `SetObjectiveTarget(성문/총통)`으로 전진 후 도착 시 공격 |
| 공심돈 적 | 발각 시 `SetRetreatTargetLocation(탈출점)`으로 도주 |
| 신기전 적 | 고정 목표를 `SetObjectiveTarget`으로 지정해 일방향 전진 |
| 총통 | 공격자 → 성문 인접 적 → 무작위 적 순으로 선택해 투사체 발사 |

## 미완료 작업

1. `BP_ChongtongCannon`, `BP_ChongtongProjectile`, 적 Blueprint와 메시/애니메이션 생성.
2. `LV_Ongseong`에 성문, 총통, 적 목표점, Actor Pool, Spawn/Wave Manager 배치.
3. 근거리 Enemy용 BT 또는 StateTree Asset 작성 및 `OnHighDetailAIChanged` 연결.
4. Android 실기기에서 풀 크기, LOD 거리, 동시 투사체·적 수를 프로파일링하고 조정.

## 최신 검증

- 2026-08-19: Editor 종료 후 `SuwonSiegeContestVREditor Win64 Development -NoHotReload` 빌드에 성공했다.
- 빌드 과정에서 UE 5.8의 `TMap<TObjectPtr<AActor>, ...>` 키 추출 타입 불일치를 수정했다.

## 에디터 MCP 상태

`http://127.0.0.1:8000/mcp` 연결은 가능하지만, 확인 시 `AgentSkillToolset`만 노출됐다.
Level/Asset/Blueprint/Compile 도구는 현재 등록되어 있지 않아 에디터 작업을 자동 수행할 수 없다.
