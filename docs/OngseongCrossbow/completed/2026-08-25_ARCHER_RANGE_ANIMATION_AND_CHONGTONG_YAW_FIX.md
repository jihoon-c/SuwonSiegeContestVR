# 궁병 사거리 · 검병 Run · 아군 총통 어셈블리 회전 수정

**완료일**: 2026-08-25

## 작업

1. 옹성 궁병의 교전 사거리를 2000cm로 변경했다.
2. 적 근접 병사가 Run 1회 후 마지막 자세에서 멈추는 문제를 검증·해결했다.
3. 아군 총통이 포신만 분리해 3축 회전하던 동작을 포신과 화차 전체의 좌우 회전으로 교체했다.

## 구현 내용

### 궁병 사거리

`AOngseongEnemyWaveManager::ArcherRange` 기본값을 3500에서 2000으로 변경했다. 본편
`LV_Ongseong`과 전투 테스트 `LV_Ongseong_CombatTest`의 `Ongseong_WaveManager` 인스턴스도
드라이런으로 기존 3500 오버라이드를 확인한 뒤 2000으로 저장했다. 목표 접근 허용 반경은
기존 계산식 `ArcherRange * 0.9`에 따라 1800cm가 된다.
`UOngseongArcherCombatComponent::EngagementRange` 기본값도 2000으로 통일했고,
`BT_EnemyArcher`의 Move To `AcceptableRadius`는 3150에서 1800으로 낮췄다. 그렇지 않으면
궁병이 새 사거리 밖에서 이동을 끝내 사격하지 못한다.

### 검병 Run 루프

`AS_MeleeRun`은 0.733초, 23키, RateScale 1.0, `loop=true`로 확인했다. 컴파일된
`ABP_EnemyMelee` 런타임 노드는 `AS_MeleeRun`을 사용하는 Sequence Player이며
`bLoopAnimation=false` 오버라이드가 없어 엔진 기본값 `true`를 사용한다. 캐릭터 메시도
Animation Blueprint 모드, `AlwaysTickPose`, 일시정지 false, 글로벌 재생률 1.0이다.

### 총통 회전

기존 `AimBarrelAtDirection()`은 `BarrelPivot`만 월드 쿼터니언으로 회전해 포신과 화차가
분리되고 Pitch/Roll까지 바뀌었다. 새 `AimAssemblyYawAtDirection()`은 포구의 현재 +Y축과
목표의 수평 방향 사이 Yaw 차이만 계산해 `HwachaBaseMesh`에 적용한다.

따라서 `ChongtongMesh`, `BarrelPivot`, `Muzzle`, 조작병 좌석 등 자식 컴포넌트의 블루프린트
상대 트랜스폼은 그대로 유지되고 포신과 화차가 한 덩어리로 좌우 회전한다. 포탄은 별도로
계산된 수직 속도를 계속 사용하므로 기존 곡사 탄도는 유지된다.

## 변경 파일

- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/OngseongEnemyWaveManager.h`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/OngseongArcherCombatComponent.h`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/ChongtongCannonActor.h`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Ongseong/ChongtongCannonActor.cpp`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Maps/LV_Ongseong.umap`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Maps/LV_Ongseong_CombatTest.umap`
- `Scripts/SetOngseongArcherRange.py`
- `Scripts/InspectEnemyAnimBlueprintRuntimeNodes.py`
- `Scripts/SetOngseongArcherBehaviorTreeRange.py`
- `Scripts/InspectOngseongArcherBehaviorTree.py`

## 테스트 결과

- `SuwonSiegeContestVR Win64 Development`: 빌드 성공
- 사거리 드라이런: 두 레벨 모두 `3500 -> 2000` 대상 확인
- 사거리 적용: 두 레벨, 변경 2건 저장 성공
- 사거리 재검증: 두 레벨 모두 `2000 -> 2000`
- Behavior Tree: Move To `AcceptableRadius 3150 -> 1800` 저장 성공
- 검병 애니메이션 진단: 시퀀스 루프/재생률/키 수/메시 Tick 설정 정상
- 컴파일된 AnimBP 덤프: Run 노드가 `AS_MeleeRun` Sequence Player이고 루프 false 오버라이드 없음

## 남은 문제

- 총통 어셈블리 좌우 회전은 실제 렌더링 PIE/HMD에서 최종 육안 확인이 필요하다.
