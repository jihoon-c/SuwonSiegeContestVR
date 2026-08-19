# 신기전 화차 VR 인터랙션 명세

구현 모듈은 `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon`이다. C++ 클래스 경로는 `/Script/GF_Singijeon`을 사용한다.

## 제공 클래스

- `ASingijeonHwachaActor`: 탄약 상태, 점화, 순차 발사를 조정하는 화차 본체
- `USingijeonAmmoSlotComponent`: 화살을 감지하고 슬롯 위치에 스냅하는 재사용 슬롯
- `ASingijeonProjectileActor`: 슬롯에 장전되고 발사되는 기본 신기전 화살
- `UFuseIgnitionComponent`: 활성 점화원이 일정 시간 접촉할 때 점화 완료
- `AIgnitionSourceActor`: 횃불용 기본 점화원
- `UTwoHandCarryComponent`: 두 모션 컨트롤러의 중점과 방향으로 액터를 운반

## 블루프린트 생성

1. `ASingijeonHwachaActor` 기반 `BP_SingijeonHwacha`를 만든다.
2. `BodyMesh`에 화차 메시를 지정한다.
3. `DefaultAmmoSlot`을 첫 발사관 위치에 놓고, 필요한 만큼 `SingijeonAmmoSlotComponent`를 추가한다.
4. 각 슬롯의 로컬 X축(Forward)을 원하는 발사 방향으로 맞춘다.
5. `Fuse`를 도화선 끝에 배치한다.
6. `ASingijeonProjectileActor` 기반 `BP_SingijeonArrow`에 화살 메시를 지정한다.
7. `AIgnitionSourceActor` 기반 `BP_SingijeonTorch`에 횃불 메시를 지정하고 `IgnitionArea`를 불꽃 위치에 둔다.

현재 BP는 `/GF_Singijeon/Gameplay/`에 생성되어 있다. 화차에는 `Wooden_Rocket_Cart`를 적용했고, 별도 신기전·횃불 메시가 프로젝트에 없어 탄약과 횃불은 플레이 검증용 기본 메시를 사용한다.

## Scenario 연결

```text
NAR_01~05 → INT_01  (Grab / Singijeon_Ammo)
NAR_06~08 → INT_02  (Custom / Hwacha_Load)
NAR_09~10 → INT_03  (Custom / Hwacha_Aim)
NAR_11~12 → INT_04  (Grab / Singijeon_Torch)
NAR_13    → INT_05  (Trigger / Torch_Ignite)
NAR_14~15 → INT_06  (Trigger / Hwacha_Fuse)
NAR_16    → INT_07  (Combat / Hwacha_Fire)
NAR_17~21 → Scenario 완료
```

전체 순서는 `DA_Scenario_Singijeon.Stages[Singijeon].Interactions[].NextInteractionID`가 단독으로 소유한다. `DT_Narration`의 각 Row는 `NextRow=None`, `AdvanceMode=Stop`으로 유지한다.

Grab은 `BP_VRPlayerPawn`이 대상 Actor의 `ScenarioInteractableComponent`에 보고한다. 장전·점화·일제 발사 완료는 `ASingijeonHwachaActor`가 자동 보고한다.

슬롯은 `SingijeonAmmunitionInterface` 구현 액터만 받으며, 도화선은 `IgnitionSourceInterface`가 활성 상태인 액터만 받는다. 따라서 메시나 액터 태그에 의존하지 않는다.

## 기존 XR Grab 연결

화살과 횃불에는 기존 `BP_GrabComponent`를 추가해 `BP_VRPlayerPawn`으로 이식된 Grab 시스템을 사용한다. `BP_SingijeonArrow`에서 `Prepare For Loading` 이벤트를 오버라이드하고, 잡혀 있다면 `BP_GrabComponent.TryRelease`를 호출한 뒤 `true`를 반환한다. 슬롯은 이 이벤트가 성공한 다음에만 화살을 스냅하므로 손을 놓는 후속 이벤트가 장전된 화살을 다시 분리하지 않는다.

화차에는 좌우 손잡이 위치에 `BP_GrabComponent` 두 개를 배치한다. 기존 Grab 컴포넌트가 화차 루트를 손에 직접 부착하지 않도록 손잡이 Grab Type을 커스텀 방식으로 설정한 후 다음 이벤트를 연결한다.

- 왼쪽 손잡이 `OnGrabbed`: `Get Held By Hand` 결과를 `TwoHandCarry.BeginGrip(Left, Hand)`에 전달
- 오른쪽 손잡이 `OnGrabbed`: `Get Held By Hand` 결과를 `TwoHandCarry.BeginGrip(Right, Hand)`에 전달
- 왼쪽 손잡이 `OnDropped`: `TwoHandCarry.EndGrip(Left, Hand)` 호출
- 오른쪽 손잡이 `OnDropped`: `TwoHandCarry.EndGrip(Right, Hand)` 호출

`Get Held By Hand`가 모션 컨트롤러가 아닌 Pawn을 반환하는 프로젝트 버전이라면, 반환된 Pawn에서 해당 `MotionControllerLeftGrip` 또는 `MotionControllerRightGrip` 컴포넌트를 넘긴다.

화살이 최소 수량 이상 장전된 `Loaded` 상태에서만 양손 운반이 활성화된다. 두 손을 모두 잡아야 이동하며, 한 손을 놓으면 즉시 멈춘다. 점화가 시작되는 순간 운반은 잠기고 손 참조도 해제된다.

## 확장 지점

- 슬롯 수: 블루프린트에서 슬롯 컴포넌트 추가
- 최소 장전 수: 화차의 `Minimum Loaded Ammunition`
- 점화 유지 시간: `Fuse.Ignition Duration`
- 탄속과 발사 간격: `Launch Speed`, `Launch Interval`
- 화살별 효과: `ASingijeonProjectileActor.OnLaunched` 오버라이드
- 횃불 점화/소화: `SetIgnitionActive`
- UI/사운드/VFX: `OnHwachaStateChanged`, `OnLoadCountChanged`, `OnVolleyLaunched`, Fuse 이벤트에 바인딩

## 상태 흐름

`Empty -> Loaded -> Igniting -> Fired`

횃불이 점화 완료 전에 도화선에서 떨어지거나 꺼지면 `Igniting -> Loaded`로 복귀한다.
