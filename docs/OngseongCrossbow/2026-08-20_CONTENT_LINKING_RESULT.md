# Ongseong 콘텐츠 연결 결과

**완료일**: 2026-08-20

## 연결한 항목

- 공용 `/Game/Gameplay/Characters/BP_EnemySoldier`를 `AEnemyCombatCharacter` 부모로 생성하고 Manny 임시 메시/애니메이션을 설정했다.
- `LV_Ongseong`의 `Enemy_ActorPool`을 공용 Enemy Blueprint, 고정 크기 8, 자동 확장 비활성으로 설정했다.
- `Projectile_ActorPool`을 생성하고 `AChongtongProjectileActor` 16개를 사전 생성하도록 설정했다.
- `JihwaGate_Main`에 Health 1000과 Ally Faction을 추가했다.
- `Chongtong_Gameplay`에 성문과 Projectile Pool을 연결했다.
- `Ongseong_WaveManager`를 추가해 1초 뒤부터 2.5초 간격, 최대 활성 적 6개, 250cm 간격으로 스폰하도록 설정했다.

## 검증 결과

- UBT `SuwonSiegeContestVREditor Win64 Development -NoHotReload`: 성공
- `SuwonSiegeContestVR` Automation: 10/10 성공, Warning 0, Error 0
- MCP Simulate PIE: 공용 Enemy 8개 사전 생성, 최대 6개 활성, 성문 목표 이동·공격과 체력 감소 확인
- 총통: Projectile Pool에서 발사 후 반환 확인. 현재 발사선이 배치 지형에 먼저 닿으므로 최종 Muzzle/충돌 튜닝 필요
- 저장 후 Level 재로딩: 참조 유지
- Map Check: Error 0, Warning 0

## 콘텐츠 담당 후속 작업

1. Content Browser에서 `/GF_OngseongCrossbow/Blueprints`의 Redirector를 Fix Up한 뒤 빈 폴더를 정리한다. 현재 Redirector의 참조 수는 0이다.
2. `BP_EnemySoldier`의 Manny를 최종 적 병사 메시·AnimBP로 교체한다.
3. 총통/포탄 Blueprint에 최종 메시, 포신 Muzzle, VFX와 SFX를 연결하고 지형 선충돌을 제거한다.
4. 근거리 BT 또는 StateTree를 작성해 AI LOD 고정밀 전환에 연결한다.
5. 웨이브 성공/실패, 성문 파괴, UI와 `ExperienceSubsystem` 보고를 구현한다.
6. Android/OpenXR 환경을 준비하고 실기기에서 Pool·LOD·동시 개체 수를 프로파일링한다.

작업 전 맵 백업은 `Saved/CodexBackups/2026-08-20_OngseongContentLink/LV_Ongseong.umap`에 있다.
