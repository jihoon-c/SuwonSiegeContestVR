# 작업

Shared Gameplay 전투 기반 C++ 구현

# 구현 내용

- `UCombatFactionComponent`: Neutral / Player / Ally / Enemy 진영과 공통 적대 판정
- `UHealthComponent`: 체력, 회복, 피격, 사망 이벤트. 소유 Actor의 제거·연출은 담당하지 않음
- `FCombatDamageSpec`, `IDamageReceiverInterface`, `UCombatDamageLibrary`: 구체 캐릭터 검사 없이 적용하는 데미지 계약
- `ACombatCharacter`, `AEnemyCombatCharacter`, `AAllyCombatCharacter`: 재사용 가능한 비플레이어 전투 Character 기반
- `AGameplayProjectileActor`: 수명과 충돌, 발사, Impact Event, 진영 기반 피해를 제공하는 추상 투사체 기반

# 변경 파일

- `Source/SuwonSiegeContestVR/Public/Gameplay/`
- `Source/SuwonSiegeContestVR/Private/Gameplay/`
- `docs/ARCHITECTURE.md`
- `docs/Main/plans/2026-08-19_SHARED_GAMEPLAY_FOUNDATION.md`

# 주요 결정 사항

- Shared Gameplay는 Core 또는 Game Feature를 참조하지 않는다.
- Player와 Ally는 같은 편이고, Enemy와만 기본적으로 적대한다. Neutral은 피해 대상이 아니다.
- Friendly-fire 우회는 `FCombatDamageSpec::bIgnoreFaction`을 명시할 때만 가능하다.
- Health Component는 사망 시 Actor를 Destroy하지 않는다. Feature별 사망 애니메이션·재사용·Scenario 보고는 `OnDeath` 구독으로 구현한다.
- 기존 신기전 전용 투사체를 공통 기반으로 즉시 교체하지 않아 현재 체험의 Blueprint/충돌 동작을 보존했다.

# 테스트 결과

- `UnrealBuildTool SuwonSiegeContestVREditor Win64 Development -NoHotReload` 성공
- Unreal Header Tool 처리 성공, C++ 17개 작업 성공
- `git diff --check` 통과

# 남은 문제

- AI Controller/Behavior Tree, 적 스폰·웨이브, 공통 UI, Gameplay Tags는 미구현이다.
- 체력·투사체 수치와 동시 생성/재사용(풀링) 정책은 Android 성능 예산과 함께 기획 결정이 필요하다.
- 각 Game Feature가 기존 공격원을 `FCombatDamageSpec`/공통 Projectile로 전환하는 별도 작업이 필요하다.
