# Singijeon (신기전) — 현재 상태

**갱신일**: 2026-08-19
**계층**: Game Feature — `GF_Singijeon`
**전체 상태**: `Status: Partial` — 핵심 VR 상호작용 C++ 구현 완료, 체험 콘텐츠와 Shared 전투 연동은 미완료

## 구현 완료

`Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon` 런타임 모듈에 다음 기능이 구현되어 있다.

| 기능 | 구현 |
|---|---|
| 화차 상태 관리 | `ASingijeonHwachaActor` (`Empty → Loaded → Igniting → Fired`) |
| 화살 장전 | `USingijeonAmmoSlotComponent`, `ISingijeonAmmunitionInterface` |
| 도화선 점화 | `UFuseIgnitionComponent`, `IIgnitionSourceInterface` |
| 횃불 기본 액터 | `AIgnitionSourceActor` |
| 횃불 점화 화로 | `AFirePitActor`, `BP_SingijeonFirePit`; 꺼진 횃불만 점화하고 `Torch_Ignite` 보고 |
| 순차 연속 발사 | 슬롯별 `LaunchInterval`, `LaunchSpeed` 적용 |
| 신기전 기본 투사체 | `ASingijeonProjectileActor` |
| 양손 화차 운반 | `UTwoHandCarryComponent` |
| 조준 가이드 | 장전 후 손잡이 표시, 양손 Grab 중 숨김, 미완료 Drop 시 복원, `Hwacha_Aim` 완료 시 해제 |
| 자동 장전 외형 | 6 x 15 ISM에 신기전 메시와 `M_Arrow01b` 머티리얼 적용 |
| 화로 불꽃 위치 | 점화 영역 `Z=102`, NS_Fire 하단 바운드 보정 원점 `Z=155` |
| Blueprint 확장 이벤트 | 상태, 장전 수, 발사 완료, 점화, 운반 상태 이벤트 |
| 이전 클래스 호환 | `SingijeonInteraction` 및 초기 게임 모듈 경로 Core Redirect |

상세 Blueprint 연결 방법은 [specs/VR_INTERACTION.md](specs/VR_INTERACTION.md)를 참고한다.

## 아직 필요한 작업

| 요소 | 상태 | 비고 |
|---|---|---|
| 화차·화살·횃불 Blueprint | Implemented | `BP_SingijeonHwacha`, `BP_SingijeonArrow`, `BP_SingijeonTorch`; 탄약·횃불 XR Grab 연결 |
| 신기전 레벨 | Partial | 플레이용 화차·탄약·횃불·PlayerStart 배치 및 Scenario 흐름 연결. VFX/SFX/표적 미완료 |
| VFX/SFX/UI | Planned | C++ Delegate에 바인딩 |
| 표적 및 점수 | Planned | 체험 전용 로직은 이 Feature에 배치 |
| Damage/Health/Faction | Planned | `GF_OngseongCrossbow`와 Shared Gameplay 구현을 공유 |
| Experience Manager | Planned | Main의 완료 인터페이스 확정 후 연결 |
| PlayerPhone 확장 | Planned | Core PlayerPhone 계약 확정 후 구현 |
| 스탠드얼론 VR 성능 검증 | Needs Verification | 동시 투사체·VFX 상한 측정 필요 |

## 계층 결정

- 화차, 장전 슬롯, 도화선, 횃불, 신기전 고유 투사체와 양손 운반은 신기전 체험 전용이므로 `GF_Singijeon`이 소유한다.
- 범용 Projectile 기반, Damage, Health, Faction과 Enemy Soldier는 Shared Gameplay가 소유한다.
- `GF_Singijeon`은 다른 Game Feature를 직접 참조하지 않는다.

## 다음 구현 순서

1. VR Preview에서 탄약 Grab → 장전 → 횃불 Grab → FirePit 점화 → 도화선 점화 → 발사 실기 검증
2. 신기전·횃불 전용 Static Mesh 교체
3. Shared Damage/Target 계약 연결
4. VFX/SFX/UI와 체험 완료 조건 연결
5. Quest/PICO 실기기 성능 측정

## 주의사항

- `ASingijeonProjectileActor`는 신기전 고유 기본 구현이다. 범용 Projectile/Damage 계약이 확정되면 그 계약을 사용하되 Shared 시스템을 이 플러그인에 복제하지 않는다.
- Game Feature는 `ExplicitlyLoaded: true`, 초기 상태는 `Registered`다. 실제 체험 진입 시 활성화 주체가 필요하다.
- 이전 `SingijeonInteraction` 플러그인은 `GF_Singijeon`에 통합되었으므로 다시 추가하지 않는다.
