# 작업

신기전 장전 화살 런타임 머터리얼 복구

# 구현 내용

- 기존 화살 텍스처 3종으로 프로젝트 소유 `M_SingijeonArrow_Runtime`을 생성했다.
- 화살 Static Mesh, 투사체 Blueprint, 화차 자동 장전 ISM, 레벨 배치 화차에 동일 머터리얼을 적용했다.
- 기존 `M_Arrow01b`의 `/InterchangeAssets/gltf/Substrate/M_GLTF` 부모 의존성을 제거했다.
- 최초 장전 뒤 XR Grab의 지연된 Release가 물리 화살 머터리얼을 다시 비우는 경로를 확인했다.
- 장전 슬롯의 Post Physics 보정에서 부착 상태와 함께 `RefreshLoadedVisual`을 호출하여, 장전 중에는 런타임 머터리얼을 계속 복구하도록 했다.
- Blueprint 오버라이드가 비어 있어도 프로젝트 소유 `M_SingijeonArrow_Runtime`을 fallback으로 불러와 모든 머터리얼 슬롯에 적용한다.
- 자동 생성되는 89개 화살은 ISM vertex factory를 사용하므로 `M_SingijeonArrow_Runtime`의 `Used with Instanced Static Meshes`를 활성화하고 머터리얼 셰이더를 재컴파일했다.
- 화차의 Construction과 자동 장전 시점에도 `MATUSAGE_InstancedStaticMeshes` 사용 가능 여부를 검사하여 잘못된 머터리얼이 다시 지정되면 오류를 남긴다.

# 변경 파일

- 신기전 화살 Material/Static Mesh/Blueprint/Level 에셋
- `Scripts/ConfigureSingijeonArrowRuntimeMaterial.py`
- `Scripts/ConfigureSingijeonAutoFillArrowGrid.py`
- `Scripts/VerifySingijeonAutoFillArrowGrid.py`
- `SingijeonAmmunitionInterface.h`
- `SingijeonProjectileActor.h/.cpp`
- `SingijeonAmmoSlotComponent.cpp`
- `SingijeonHwachaTests.cpp`
- `ConfigureSingijeonArrowRuntimeMaterial.py`

# 주요 결정 사항

Cook과 ISM 렌더링에 필요한 머터리얼은 에디터 Import 지원 플러그인 자산이 아닌 Game Feature 소유 Material로 유지한다.

# 테스트 결과

- Static Mesh, 투사체, CDO ISM, 배치 ISM의 머터리얼 참조 검증 성공
- UE 5.8 Development Editor 빌드 성공
- 지연 Release를 재현해 물리 화살 머터리얼을 `nullptr`로 바꾼 뒤, Post Physics Tick에서 머터리얼과 슬롯 부착이 모두 복구되는 회귀 테스트 성공
- GF_Singijeon 자동화 테스트 5/5 성공
- ISM 사용 플래그, 화차 CDO ISM, `LV_Singijeon` 배치 ISM의 머터리얼 검증 성공

# 남은 문제

Quest 3 실기기에서 최종 색감과 노멀 강도를 확인할 수 있다.
