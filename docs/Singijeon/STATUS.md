# Singijeon (신기전) — 현재 상태

**갱신일**: 2026-08-21
**계층**: Game Feature — `GF_Singijeon`
**전체 상태**: `Status: Partial` — 핵심 VR 상호작용 C++ 구현 완료, 체험 콘텐츠와 Shared 전투 연동은 미완료

## 구현 완료

`Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon` 런타임 모듈에 다음 기능이 구현되어 있다.

| 기능 | 구현 |
|---|---|
| 화차 상태 관리 | `ASingijeonHwachaActor` (`Empty → Loaded → Igniting → Fired`) |
| 화살 장전 | `USingijeonAmmoSlotComponent`, `ISingijeonAmmunitionInterface`; 지연 Grab Release 후에도 Post Physics에서 슬롯 Attach·로컬 Transform 복구 |
| 도화선 점화 | `UFuseIgnitionComponent`, `IIgnitionSourceInterface`; 뒤쪽 중앙 발광 `FuseGuide`, 24cm 접촉 반경과 진행 중 `FuseIgnitionEffect` |
| 횃불 기본 액터 | `AIgnitionSourceActor` |
| 횃불 점화 화로 | `AFirePitActor`, `BP_SingijeonFirePit`; 꺼진 횃불만 점화하고 `Torch_Ignite` 보고 |
| 10초 랜덤 순차 발사 | Fuse 완료 요청을 나레이션 동안 보존, Fire 단계에서 자동 시작, 한 발마다 실제/ISM 화살과 장전 수 감소 |
| 신기전 기본 투사체 | `ASingijeonProjectileActor` |
| 한 손 화차 운반 | `UTwoHandCarryComponent`; 보이는 좌·우 원통 전체 Bounds Grab, 바닥 접촉에 막히지 않는 non-sweep 직접 추종, 두 손 전환 지원 |
| 화차 이동 가이드 | 보이는 원통과 native 지점 모두 `VRGrab`, 손 이동량 직접 추종, 250cm 전방 화차 홀로그램, 55cm 반경 도착 시 스냅·`Hwacha_Aim` 완료 |
| 적군 돌진 최적화 | 실행 전 EditorOnly 편대 프리뷰 45명; 런타임은 피격 Actor 3명 + 일반 Skeletal Mesh 프록시 42명, 8개 포즈 리더 공유·LOD1·10Hz 이동; Samurai 175cm 정규화; GPU 인스턴스는 선택 옵션 |
| 적군 절차 접근 게이트 | 장전 35% → 화차 배치 60% → 점화 82% → 발사 95%, 전탄 종료 후 생존자만 최종 도착 허용 |
| 인터랙션 순서 게이트 | 현재 Scenario Target/Type이 일치할 때만 장전·FirePit 점화·Fuse·발사 물리 상태 변경 허용 |
| 자동 장전 외형 | 6 x 11 ISM을 `RackRoot` 로컬 자식으로 고정, 발사마다 Remove/Render 갱신, 물리·ISM·Spawn 화살에 `M_SingijeonArrow_Runtime` 재적용 |
| 화로 불꽃 위치 | FirePit 메시 CPU 접근 허용, 부모 상대좌표 강제 및 월드 `Z≈210` 보정 |
| Blueprint 확장 이벤트 | 상태, 장전 수, 발사 완료, 점화, 운반 상태 이벤트 |
| 이전 클래스 호환 | `SingijeonInteraction` 및 초기 게임 모듈 경로 Core Redirect |

상세 Blueprint 연결 방법은 [specs/VR_INTERACTION.md](specs/VR_INTERACTION.md)를 참고한다.

## 아직 필요한 작업

| 요소 | 상태 | 비고 |
|---|---|---|
| 화차·화살·횃불 Blueprint | Implemented | `BP_SingijeonHwacha`, `BP_SingijeonArrow`, `BP_SingijeonTorch`; 탄약·횃불 XR Grab 연결 |
| 신기전 레벨 | Partial | 플레이용 화차·탄약·횃불·PlayerStart 배치 및 Scenario 흐름 연결. VFX/SFX/표적 미완료 |
| VFX/SFX/UI | Partial | FirePit/Fuse Niagara와 Interaction Guide 구현. 발사·피격 SFX 및 결과 UI는 Delegate 연결 대기 |
| 표적 및 점수 | Planned | 체험 전용 로직은 이 Feature에 배치 |
| Damage/Health/Faction | Implemented | 발사된 신기전 투사체가 Health 대상에 표준 Damage를 1회 전달하고 Impact Delegate 제공 |
| Experience Manager | Implemented | `DA_Experience_Singijeon`과 Scenario 완료 후 Main 복귀 연결 |
| PlayerPhone 확장 | Planned | Core PlayerPhone 계약 확정 후 구현 |
| 스탠드얼론 VR 성능 검증 | Needs Verification | 동시 투사체·VFX 상한 측정 필요 |

## 계층 결정

- 화차, 장전 슬롯, 도화선, 횃불, 신기전 고유 투사체와 화차 운반은 신기전 체험 전용이므로 `GF_Singijeon`이 소유한다.
- 범용 Projectile 기반, Damage, Health, Faction과 Enemy Soldier는 Shared Gameplay가 소유한다.
- `GF_Singijeon`은 다른 Game Feature를 직접 참조하지 않는다.

## 다음 구현 순서

1. VR Preview에서 탄약 Grab → 장전 → 횃불 Grab → FirePit 점화 → 도화선 점화 → 발사 실기 검증
2. 신기전·횃불 전용 Static Mesh 교체
3. 발사·피격 SFX와 결과 UI 연결
4. 역사 에셋과 타격 VFX 교체
5. Quest/PICO 실기기 성능 측정

## 주의사항

- `ASingijeonProjectileActor`는 신기전 고유 기본 구현이다. 범용 Projectile/Damage 계약이 확정되면 그 계약을 사용하되 Shared 시스템을 이 플러그인에 복제하지 않는다.
- Game Feature는 `ExplicitlyLoaded: true`, 초기 상태는 `Registered`다. 실제 체험 진입 시 활성화 주체가 필요하다.
- 이전 `SingijeonInteraction` 플러그인은 `GF_Singijeon`에 통합되었으므로 다시 추가하지 않는다.
