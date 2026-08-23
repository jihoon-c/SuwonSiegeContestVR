# 작업

FirePit 불꽃 위치 및 신기전 자동 장전 머티리얼 재발 방지 수정

# 구현 내용

- `Geometric_Fire_Pit` Static Mesh의 `Allow CPU Access`를 활성화했다.
- `AFirePitActor`가 `FireEffect` Niagara 컴포넌트를 구성 시점과 플레이 시작 시점에 다시 부모 상대좌표로 배치한다.
- FireEffect의 로컬 `Z=280` 보정은 FirePit 메시 피벗 오프셋을 고려해 레벨 월드 `Z≈210`에 배치된다.
- 화차 자동 장전 ISM이 여러 메시 슬롯을 가질 때도 머터리얼을 모든 슬롯에 적용하도록 수정했다.
- 후속 점검에서 `M_Arrow01b`의 Interchange GLTF Substrate 부모 의존성을 제거하고 프로젝트 소유 `M_SingijeonArrow_Runtime`으로 교체했다.
- Blueprint와 `BP_SingijeonHwacha_Playable` 레벨 인스턴스의 화살 메시·머티리얼 오버라이드를 재저장했다.

# 변경 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Interaction/FirePitActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Singijeon/SingijeonHwachaActor.cpp
Plugins/GameFeatures/GF_Singijeon/GF_Singijeon.Build.cs
Plugins/GameFeatures/GF_Singijeon/GF_Singijeon.uplugin
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonFirePit.uasset
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonHwacha.uasset
Plugins/GameFeatures/GF_Singijeon/Content/Asset/FirePit/Geometric_Fire_Pit.uasset
Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap
Scripts/ConfigureSingijeonFirePitFlow.py
Scripts/ConfigureSingijeonAutoFillArrowGrid.py
Scripts/VerifySingijeonFirePitFlow.py
Scripts/VerifySingijeonAutoFillArrowGrid.py
Scripts/ApplySingijeonFirePitAndAmmoVisualFixes.py
Scripts/VerifySingijeonFirePitAndAmmoVisualFixes.py
Scripts/InspectSingijeonFirePitAndAmmoVisuals.py
```

# 주요 결정 사항

- 레벨 인스턴스의 Blueprint 컴포넌트 오버라이드는 재구성 시 이전 위치로 복원될 수 있으므로, C++ 액터에서 런타임과 구성 시점 모두 변환을 보정한다.
- 화살 메시의 슬롯 수에 의존하지 않도록 Auto Fill 머티리얼을 모든 슬롯에 적용한다.
- 자동 장전 ISM, 물리 화살 Static Mesh, 발사 투사체가 같은 독립 Material을 사용한다.

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- FirePit 에셋 검증: CPU 접근, 부모 추종, 월드 `Z≥200`, Scenario 연결 통과
- 자동 장전 에셋 검증: 메시 및 머티리얼 오버라이드 통과
- 전체 `SuwonSiegeContestVR` 자동화 테스트 15/15 성공

# 남은 문제

- Quest 3 실기기에서 화염 크기와 밝기는 최종 미세 조정이 필요할 수 있다.
