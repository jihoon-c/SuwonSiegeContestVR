# 신기전 화차 VR 인터랙션 명세

구현 모듈은 `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon`이다. C++ 클래스 경로는 `/Script/GF_Singijeon`을 사용한다.

## 제공 클래스

- `ASingijeonHwachaActor`: 탄약 상태, 점화, 순차 발사를 조정하는 화차 본체
- `USingijeonAmmoSlotComponent`: 화살을 감지하고 슬롯 위치에 스냅하는 재사용 슬롯
- `ASingijeonProjectileActor`: 슬롯에 장전되고 발사되는 기본 신기전 화살
- `UFuseIgnitionComponent`: 활성 점화원이 일정 시간 접촉할 때 점화 완료
- `AIgnitionSourceActor`: 횃불용 기본 점화원
- `AFirePitActor`: 꺼진 횃불을 점화하고 `Torch_Ignite` 성공을 보고하는 화로
- `UTwoHandCarryComponent`: 두 모션 컨트롤러의 중점과 방향으로 액터를 운반

## 블루프린트 생성

1. `ASingijeonHwachaActor` 기반 `BP_SingijeonHwacha`를 만든다.
2. `BodyMesh`에 화차 메시를 지정한다.
3. `DefaultAmmoSlot`을 첫 발사관 위치에 놓고, 필요한 만큼 `SingijeonAmmoSlotComponent`를 추가한다.
4. 각 슬롯의 로컬 X축(Forward)을 원하는 발사 방향으로 맞춘다.
5. `Fuse`를 도화선 끝에 배치한다.
6. `ASingijeonProjectileActor` 기반 `BP_SingijeonArrow`에 화살 메시를 지정한다.
7. `AIgnitionSourceActor` 기반 `BP_SingijeonTorch`에 횃불 메시를 지정하고 `IgnitionArea`를 불꽃 위치에 둔다.
8. `AFirePitActor` 기반 `BP_SingijeonFirePit`에 `Geometric_Fire_Pit` 메시와 불 VFX를 지정한다.

현재 BP는 `/GF_Singijeon/Gameplay/`에 생성되어 있다. 화차는 `/GF_Singijeon/Asset/Hwacha/hwacha/StaticMeshes/hwacha`, 화살은 `/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb` 메시를 사용한다.

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

횃불은 `bIgnitionActive=false`로 시작한다. `BP_SingijeonFirePit`의 `IgnitionArea`에 횃불이 닿으면 횃불이 활성화되고 기존 Niagara 불꽃이 켜지며 `Torch_Ignite / Trigger`가 완료된다. 비활성 횃불은 `UFuseIgnitionComponent`가 거부하므로 FirePit을 거치지 않고 화차 도화선을 점화할 수 없다.

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

## 화살 6 x 15 자동 장전

- 플레이어가 첫 화살 한 발을 `DefaultAmmoSlot`에 장전하면 같은 Static Mesh의 Instance 89개가 자동 생성된다.
- 실제 화살 1발과 ISM 89개를 합쳐 총 90발로 계산한다.
- 화살을 장전 취소하면 자동 생성된 Instance도 제거된다.
- 발사할 때 Instance는 첫 화살과 같은 Projectile Actor 클래스로 한 발씩 변환되어 `LaunchInterval` 간격으로 발사된다.
- `Auto Fill Rows`, `Auto Fill Columns`, 간격, Offset과 Mesh는 화차 Blueprint에서 조절할 수 있다.
- `Auto Fill Arrow Material`이 지정되면 모든 자동 장전 Instance에 우선 적용한다. 비어 있으면 첫 물리 화살의 머티리얼을 복사한다.

## 시각 피드백 설정

- `BP_SingijeonFirePit.IgnitionArea`는 화로 상단 `Z=102`, `FireEffect` 원점은 `NS_Fire`의 하단 바운드를 보정한 `Z=155`에 있다.
- 화차가 장전되어 조준 인터랙션이 필요해지면 `LeftHandleHighlight`, `RightHandleHighlight`가 켜진다.
- 한 손만 잡았을 때는 가이드가 유지된다. 양손을 모두 올바르게 잡으면 손 위치 안내가 끝났으므로 숨는다.
- 이동을 완료하기 전에 손을 놓으면 다시 켜진다. 최초 장전 위치에서 30cm 이동 또는 10도 회전하면 `Hwacha_Aim / Custom`을 보고하고 최종 해제된다.
- 목표 지점 Trigger 등 별도의 성공 판정이 있다면 `CompleteAimInteraction()`을 호출한다.
- 손잡이 표시 위치가 에셋과 어긋나면 Blueprint 컴포넌트의 Transform만 조정한다. 기본 위치는 `(-47, -43, 61)`, `(-47, 43, 61)`이다.
- 표시 기능 자체를 끄려면 화차의 `Enable Aim Guide Highlight`를 비활성화한다. 자동 완료 기준은 `Aim Completion Distance`, `Aim Completion Yaw Degrees`로 조절한다.

## 확장 지점

- 슬롯 수: 블루프린트에서 슬롯 컴포넌트 추가
- 최소 장전 수: 화차의 `Minimum Loaded Ammunition`
- 점화 유지 시간: `Fuse.Ignition Duration`
- 탄속과 발사 간격: `Launch Speed`, `Launch Interval`
- 화살별 효과: `ASingijeonProjectileActor.OnLaunched` 오버라이드
- 횃불 점화/소화: `SetIgnitionActive`
- FirePit 점화 범위: `BP_SingijeonFirePit.IgnitionArea`
- UI/사운드/VFX: `OnHwachaStateChanged`, `OnLoadCountChanged`, `OnVolleyLaunched`, Fuse 이벤트에 바인딩

## 적군 돌진 Wave

`LV_Singijeon`에는 `ASingijeonEnemyWaveActor` 기반 `Singijeon_EnemyWave`가 한 개
배치되어 있다. 개별 적군 Spline이나 AIController는 사용하지 않는다.

```text
화차 첫 장전
  → Enemy Wave 자동 시작
  → SpawnVolume에서 화차까지 Nav 경로 1회 계산
  → 45명 / 3개 소대 돌진
  → 화차 OnVolleyLaunched
  → 논리 피격 판정
  → Defeated 또는 ReachedTarget
```

- `Enemy Count`: 기본 45명
- `Max Interactive Enemies`: 실제 Shared Enemy Actor 최대 10명
- 나머지 35명: 충돌과 개별 Tick이 없는 HISM 시각 대리체
- `Target Actor`: Level의 `BP_SingijeonHwacha`, 비어 있으면 자동 탐색
- `Start When Hwacha Loaded`: 첫 화살 장전 완료 시 시작
- `Proxy Update Interval`: 기본 0.0667초(약 15Hz)
- `Volley Casualty Fraction`: 기본 1.0, 90발 일제 사격으로 남은 Wave 처리

경로는 Spawn과 Target 사이 NavMesh를 한 번만 계산한다. NavMesh가 없거나 경로를
찾지 못해도 직선 경로로 자동 fallback한다. 레벨 작업자는 적마다 경로를 만들지 않고
`Singijeon_EnemyWave` Actor의 위치만 적군 시작 지점으로 옮기면 된다.

현재 HISM에는 임시 `SM_MannequinTarget`, 실체 병사에는 Manny와 Jog Animation을
사용한다. 최종 역사 병사 VAT Mesh/Material이 준비되면 `Proxy Mesh`만 교체하며 Wave
로직은 변경하지 않는다. Proxy Material은 Per Instance Custom Data 0에 재생 시작 위상,
1에 속도 배율을 받을 수 있다.

## 상태 흐름

`Empty -> Loaded -> Igniting -> Fired`

횃불이 점화 완료 전에 도화선에서 떨어지거나 꺼지면 `Igniting -> Loaded`로 복귀한다.
