# 작업

화차 운반 중 장전 화살 추종, 순차 발사 외형 제거, 최초 물리 화살 머터리얼 복구

# 구현 내용

- 탄약 슬롯은 장전 중에만 Post Physics Tick으로 물리 화살의 슬롯 Attach와 로컬 위치·회전을 보정한다.
- XR Grab의 지연된 Release가 장전 후 화살을 분리해도 즉시 슬롯에 다시 고정된다.
- 자동 채움 ISM을 `RackRoot`의 Identity 로컬 자식으로 정규화하고 절대 위치·회전·스케일을 사용하지 않는다.
- ISM 발사는 Instance 제거 성공 후 Render State를 갱신하고 다음 장전 수를 계산한다.
- 발사체는 `ProjectileMaterialOverride`를 가지며 장전·언로드·발사 때 모든 메시 슬롯에 `M_SingijeonArrow_Runtime`을 다시 적용한다.
- 발사된 화살 Actor에는 기본 12초 수명을 적용해 충돌 후 월드에 영구 잔류하지 않게 했다.
- 화살 Static Mesh, 화살 BP, 화차 BP, 배치 화차와 배치 화살 에셋에도 같은 머터리얼과 Transform 설정을 저장했다.

# 변경 파일

- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Public/Singijeon/SingijeonAmmoSlotComponent.h`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Singijeon/SingijeonAmmoSlotComponent.cpp`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Public/Singijeon/SingijeonProjectileActor.h`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Singijeon/SingijeonProjectileActor.cpp`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Singijeon/SingijeonHwachaActor.cpp`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonHwachaTests.cpp`
- `Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonArrow.uasset`
- `Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonHwacha.uasset`
- `Plugins/GameFeatures/GF_Singijeon/Content/Asset/Arrow/arrowb/StaticMeshes/arrowb.uasset`
- `Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap`
- 관련 구성·진단·검증 스크립트

# 주요 결정 사항

- Grab 시스템 전체를 변경하지 않고 장전 슬롯이 자신의 소유 상태를 보장하도록 했다.
- 89개 자동 화살은 개별 Actor로 바꾸지 않고 ISM을 유지하여 Quest 성능을 보존했다.
- 머터리얼은 BP 설정만 믿지 않고 발사체 생명주기의 상태 변경 직후 C++에서 재적용한다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공
- `SuwonSiegeContestVR.GF_Singijeon` 자동화 테스트 4/4 성공
- 강제 Detach 후 슬롯 재부착 성공
- 화차 운반 시 물리 화살과 ISM 화살이 화차와 동일 Delta로 이동
- 순차 발사 장전 수 `90 → 89 → 88 → ... → 0` 검증
- BP·Static Mesh·배치 Actor 머터리얼 및 `RackRoot` 로컬 부착 검증 Commandlet 성공

# 남은 문제

Quest 3 실기에서 90발 발사 중 외형 제거와 머터리얼 색감을 최종 확인한다.
