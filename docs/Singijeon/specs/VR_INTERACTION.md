# 신기전 화차 VR 인터랙션 명세

구현 모듈은 `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon`이다. C++ 클래스 경로는 `/Script/GF_Singijeon`을 사용한다.

## 제공 클래스

- `ASingijeonHwachaActor`: 탄약 상태, 점화, 순차 발사를 조정하는 화차 본체
- `USingijeonAmmoSlotComponent`: 화살을 감지하고 슬롯 위치에 스냅하는 재사용 슬롯
- `ASingijeonProjectileActor`: 슬롯에 장전되고 발사되는 기본 신기전 화살
- `UFuseIgnitionComponent`: 활성 점화원이 일정 시간 접촉할 때 점화 완료
- `AIgnitionSourceActor`: 횃불용 기본 점화원
- `AFirePitActor`: 꺼진 횃불을 점화하고 `Torch_Ignite` 성공을 보고하는 화로
- `UTwoHandCarryComponent`: 좌우 어느 한 손 또는 두 손의 이동량으로 액터를 운반

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

물리 상태 변경도 이 순서를 강제한다. 현재 Running Interaction의 Target/Type이 정확히 일치하지 않으면 `Hwacha_Load` 장전, `Torch_Ignite` 횃불 점화, `Hwacha_Fuse` 도화선 점화, `Hwacha_Fire` 발사가 시작되지 않는다. 조기 접촉은 완료 처리만 무시하는 것이 아니라 Attach, 점화 상태, VFX, 발사 상태를 모두 그대로 유지한다. 올바른 단계가 시작된 뒤 물체를 접촉 영역에서 뺐다가 다시 넣어야 한다.

Grab은 `BP_VRPlayerPawn`이 대상 Actor의 `ScenarioInteractableComponent`에 보고한다. 장전·점화·일제 발사 완료는 `ASingijeonHwachaActor`가 자동 보고한다.

횃불은 `bIgnitionActive=false`로 시작한다. `BP_SingijeonFirePit`의 `IgnitionArea`에 횃불이 닿으면 횃불이 활성화되고 기존 Niagara 불꽃이 켜지며 `Torch_Ignite / Trigger`가 완료된다. 비활성 횃불은 `UFuseIgnitionComponent`가 거부하므로 FirePit을 거치지 않고 화차 도화선을 점화할 수 없다.

횃불의 Niagara는 점화 접촉 순간 새로 초기화하지 않는다. `AIgnitionSourceActor.BeginPlay`에서 렌더링을 끈 채 1 Tick 사전 시뮬레이션하고 Pause 상태로 유지한다. 점화 시에는 준비된 인스턴스의 Pause와 렌더링만 해제하며, 소화 시에는 파괴하지 않고 다시 Pause/숨김 처리한다. 기본 컬링 거리는 1500cm이고 Niagara scalability를 허용한다.

슬롯은 `SingijeonAmmunitionInterface` 구현 액터만 받으며, 도화선은 `IgnitionSourceInterface`가 활성 상태인 액터만 받는다. 따라서 메시나 액터 태그에 의존하지 않는다.

## 기존 XR Grab 연결

화살과 횃불에는 기존 `BP_GrabComponent`를 추가해 `BP_VRPlayerPawn`으로 이식된 Grab 시스템을 사용한다. `BP_SingijeonArrow`에서 `Prepare For Loading` 이벤트를 오버라이드하고, 잡혀 있다면 `BP_GrabComponent.TryRelease`를 호출한 뒤 `true`를 반환한다. 슬롯은 이 이벤트가 성공한 다음에만 화살을 스냅하므로 손을 놓는 후속 이벤트가 장전된 화살을 다시 분리하지 않는다.

`USingijeonAmmoSlotComponent`는 장전 중에만 Post Physics Tick을 켜고 물리 화살의 부모와 로컬 위치·회전을 슬롯으로 강제한다. XR Grab의 지연된 Release가 뒤늦게 Actor를 분리하거나 머터리얼을 비워도 같은 프레임 끝에 다시 슬롯에 고정하고 `RefreshLoadedVisual`을 호출한다. `ASingijeonProjectileActor`는 이 호출과 장전·언로드·발사 시 `Projectile Material Override`를 모든 슬롯에 다시 적용하며, 값이 비어 있으면 프로젝트 소유 `M_SingijeonArrow_Runtime`을 fallback으로 사용한다.

화차는 C++ 기본 컴포넌트 `LeftHandleGrabPoint`, `RightHandleGrabPoint`와 보이는 원통형 `LeftHandleHighlight`, `RightHandleHighlight`를 제공한다. 네 컴포넌트 모두 `VRGrab` 대상으로 동작하며, 특히 보이는 원통은 전체 Bounds에서 Grab 거리를 계산한다. 원통의 중앙이 아닌 끝부분을 잡아도 Grab/Release가 화차의 `TwoHandCarry`에 자동 전달되므로 별도 Blueprint 이벤트 그래프 연결은 필요 없다.

화살이 최소 수량 이상 장전된 `Loaded` 상태에서만 운반이 활성화된다. 왼쪽 또는 오른쪽 손잡이 중 하나만 잡아도 화차가 해당 손의 이동량을 따라간다. 다른 손을 추가하거나 한 손을 놓으면 현재 위치에서 기준점을 다시 계산해 화차가 튀지 않는다. 점화가 시작되는 순간 운반은 잠기고 손 참조도 해제된다.

화차의 직접 손 추종은 `TwoHandCarry.Sweep Movement=false`를 사용한다. 화차 본체가 바닥과 이미 접촉한 상태에서 Sweep 이동하면 매 프레임 이동량이 바닥 충돌에 막히기 때문이다. 필요 시 다른 운반 액터에서는 이 옵션을 켤 수 있다.

## 화살 6 x 15 자동 장전

- 플레이어가 첫 화살 한 발을 `DefaultAmmoSlot`에 장전하면 같은 Static Mesh의 Instance 89개가 자동 생성된다.
- `AutoLoadedArrowInstances`는 절대 Transform을 사용하지 않고 `RackRoot`의 Identity 로컬 자식으로 유지되므로 화차 이동·회전을 동일하게 따른다.
- 실제 화살 1발과 ISM 89개를 합쳐 총 90발로 계산한다.
- 화살을 장전 취소하면 자동 생성된 Instance도 제거된다.
- 재진입 가드와 1회 성공 보고 상태로 같은 슬롯 이벤트가 반복되어도 6 x 15 배열을 중복 생성하지 않는다.
- 발사할 때 실제 화살과 Instance 전체를 섞은 무작위 순서로 한 발씩 발사한다. 첫 발부터 마지막 발까지 `Volley Duration`(기본 10초)에 맞춰 균등 배분한다.
- 한 발이 발사될 때마다 대응하는 실제 화살 또는 ISM Instance 하나를 제거하고 장전 수를 `90 → 89 → ... → 0`으로 갱신한다.
- ISM 발사는 Instance 월드 Transform을 읽은 뒤 `RemoveInstance` 성공을 확인하고 Render State를 갱신한 다음 발사체를 Spawn한다. 제거 실패 시 장전 수를 감소시키지 않는다.
- 발사 방향은 슬롯의 임의 Forward가 아니라 화살 메시의 `Arrow Tip Local Axis`를 월드 방향으로 변환해 결정한다. 현재 신기전 화살 메시의 화살촉 축은 로컬 `-X`다. 발사체는 비행 속도에 맞춰 이 화살촉 축을 회전시키며, 화차와 같은 일제 발사 화살끼리는 이동 충돌을 무시한다.
- 발사된 물리 화살 Actor는 기본 12초 후 제거되어 90발 Volley의 충돌 잔해가 월드에 계속 누적되지 않는다.
- `Auto Fill Rows`, `Auto Fill Columns`, 간격, Offset과 Mesh는 화차 Blueprint에서 조절할 수 있다.
- `Auto Fill Arrow Material`이 지정되면 모든 자동 장전 Instance에 우선 적용한다. 비어 있으면 첫 물리 화살의 머티리얼을 복사한다.
- 현재는 `/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_SingijeonArrow_Runtime`을 사용한다. 기존 `M_Arrow01b`는 Interchange GLTF Substrate 부모에 의존하므로 런타임 ISM용으로 사용하지 않는다.

## 레벨 배치용 화살 상자와 발사 효과음

- `/GF_Singijeon/Gameplay/Props/BP_SingijeonArrowBox_Instanced`는 레벨에 일반 Actor처럼 배치하는 표시용 화살 상자다.
- 나무 트레이와 신기전 화살 90개(6 x 15)를 하나의 `ArrowInstances` ISM으로 렌더링한다. 화살 Instance는 충돌과 개별 그림자를 끄므로 보급품/배경 장식에 사용한다. 이 Actor는 화차 탄약 수와 연결되지 않는다.
- `BP_SingijeonHwacha` 또는 레벨에 배치한 화차 Actor의 **Singijeon > Launch > Audio**에서 `Arrow Launch Sound`에 Sound Wave 또는 Sound Cue를 지정한다.
- `Arrow Launch Sound Volume`, `Arrow Launch Sound Pitch Min/Max`로 화살마다 재생되는 소리의 크기와 피치 랜덤 범위를 조절한다. 기본 Sound는 비어 있어 기존 레벨의 음향은 바뀌지 않는다.
- 물리 장전 화살과 자동 장전 ISM 화살 모두 성공적으로 발사된 직후 같은 설정을 사용한다.

## 시각 피드백 설정

- `BP_SingijeonFirePit.IgnitionArea`는 화로 상단 로컬 `Z=102`에 있다. `FireEffect`는 메시 피벗 `-68.664cm`와 `NS_Fire` 바운드를 함께 보정한 로컬 `Z=280`이며, 기본 레벨에서는 월드 `Z≈210`에 표시된다.
- 화차가 장전되면 `MoveTargetMarker`가 시작 위치 기준 로컬 X축 250cm 앞에 화차 전체 형상의 청록 반투명 홀로그램으로 나타난다. BeginPlay에서 분리되므로 화차를 움직여도 목표는 따라오지 않는다.
- 화차 중심이 마커의 `Move Target Acceptance Radius`(기본 55cm) 안에 들어오면 `Hwacha_Aim / Custom`을 보고하고 마커를 숨긴다.
- 성공 시 화차를 홀로그램 Transform에 정확히 스냅하고 운반을 잠근다.
- `LeftHandleHighlight`, `RightHandleHighlight`는 잡을 손잡이를 안내하며 한 손 운반이 시작되면 숨고 미완료 Drop 시 복원된다.
- `Fuse`는 화차 뒤쪽 중앙, 화차 로컬 좌표 `(-35, 0, 60)`에 있다. 화차 배치를 완료하면 작은 발광 구체 `FuseGuide`가 표시된다.
- 점화된 횃불의 손잡이가 아니라 불꽃이 있는 상단 `IgnitionArea`를 `FuseGuide`에 대고 기본 0.75초 유지한다. `Fuse` 감지 반경은 24cm이며 승인되면 `FuseIgnitionEffect(NS_Fire)`가 점화 유지 시간 동안 켜지고 취소·완료 시 꺼진다.
- `Hwacha_Fuse` 완료 직후 다음 나레이션이 재생되는 동안에는 발사 요청을 보존한다. Scenario가 `Hwacha_Fire` 단계에 도달하면 별도 재접촉 없이 자동 발사한다.
- 적군 쪽에 보이던 다수 원형은 과거 HISM 대리 적군의 `SM_MannequinTarget` 형상이었으며 현재는 제거되어 풀 캐릭터 GPU 인스턴스로 교체됐다.
- 목표 지점 Trigger 등 별도의 성공 판정이 있다면 `CompleteAimInteraction()`을 호출한다.
- 목표 위치는 Blueprint의 `MoveTargetMarker.RelativeTransform`, 표시 여부는 `Show Move Target Marker`, 도착 범위는 `Move Target Acceptance Radius`로 조절한다.

## 확장 지점

- 슬롯 수: 블루프린트에서 슬롯 컴포넌트 추가
- 최소 장전 수: 화차의 `Minimum Loaded Ammunition`
- 점화 유지 시간: `Fuse.Ignition Duration`
- 탄속과 전탄 발사 시간: `Launch Speed`, `Volley Duration`
- 화살별 효과: `ASingijeonProjectileActor.OnLaunched` 오버라이드
- 화살 피해: 발사 후 Health 대상 첫 충돌에 `ImpactDamage`를 표준 Damage로 1회 전달, `OnProjectileImpact`에서 VFX/SFX 연결
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
- `Max Interactive Enemies`: 실제 Shared Enemy Actor 최대 3명
- 나머지 42명: `UInstancedSkinnedMeshComponent`가 그리는 풀 스켈레탈 캐릭터
- `Proxy Update Interval`: 0.1초(10Hz), `Proxy Cull Distance`: 5000~12000cm
- GPU 캐릭터는 Samurai용 Rifle Jog의 12개 재생 속도/시작 위상을 공유한다.
- 재생 속도는 인스턴스별 `0.86~1.14` 범위로 분산되어 동일한 보폭 반복을 줄인다.
- `SKM_Low_Poly_Samurai_VR`은 3개 LOD를 가지며 Wave에서는 최소 LOD 1을 강제한다.
- 위치·진행 간격·회전·크기·속도는 충돌 없는 셀 범위에서 Seed 기반으로 랜덤화된다.
- 원거리 캐릭터는 화면 크기 0.006 미만에서 애니메이션만 정지하며 캐릭터 실루엣은 유지한다.
- 실제 Shared Enemy Actor는 동적 그림자를 사용하지 않는다.
- `Target Actor`: Level의 `BP_SingijeonHwacha`, 비어 있으면 자동 탐색
- `Start When Hwacha Loaded`: 첫 화살 장전 완료 시 시작
- `Volley Casualty Fraction`: 기본 1.0, 90발 일제 사격으로 남은 Wave 처리

### 절차별 적군 접근 제한

적군의 공용 Route와 각 편대 최종 위치는 유지하면서 다음 화차 절차가 끝나기 전에는 지정 비율보다 가까이 접근하지 못한다.

```text
첫 화살 장전       → Route 35%까지
화차 목표 배치 완료 → Route 60%까지
도화선 점화 시작    → Route 82%까지
신기전 발사 중      → Route 95%까지
전탄 발사 완료      → 생존자만 최종 위치까지
```

각 상한에 도착하면 Enemy Wave Tick도 중지되고 다음 절차 이벤트에서 다시 활성화된다. `Singijeon_EnemyWave` Actor의 `Procedure Gates`에서 기능 사용 여부와 `Loaded/Aimed/Igniting/Firing Approach Limit`을 조절한다. 절차 취소 시 이미 전진한 적군은 뒤로 순간이동하지 않는다.

경로는 Spawn과 Target 사이 NavMesh를 한 번만 계산한다. NavMesh가 없거나 경로를
찾지 못해도 직선 경로로 자동 fallback한다. 레벨 작업자는 적마다 경로를 만들지 않고
`Singijeon_EnemyWave` Actor의 위치만 적군 시작 지점으로 옮기면 된다.

원형이 포함된 `SM_MannequinTarget`은 더 이상 렌더링하지 않는다. 전경 3명과 원거리
42명 모두 `SKM_Low_Poly_Samurai_VR`을 사용한다. Manny/Quinn용
`MF_Rifle_Jog_Fwd`는 IK Rig와 `RTG_MannyRifle_To_LowPolySamurai`를 통해 Samurai
Skeleton용 `MF_Rifle_Jog_Fwd_Samurai`로 리타겟했다. 원거리 병력은
`DA_SingijeonSamuraiRifleRun_GPU`로 GPU 인스턴싱하며 Wave 로직은 변경하지 않는다.

## 상태 흐름

`Empty -> Loaded -> Igniting -> Fired`

횃불이 점화 완료 전에 도화선에서 떨어지거나 꺼지면 `Igniting -> Loaded`로 복귀한다.
