# 목적

상태: 완료

신기전 점화 순서를 `꺼진 횃불 집기 → FirePit에서 횃불 점화 → 화차 도화선 점화`로 강제한다.

# 현재 상태

- DA에는 `INT_04 → INT_05(Torch_Ignite) → INT_06(Hwacha_Fuse)` 순서가 있다.
- 횃불 `AIgnitionSourceActor`는 기본 점화 상태라 FirePit 없이 도화선을 점화할 수 있다.
- FirePit 메시 에셋은 있으나 점화 Actor와 레벨 배치가 없다.

# 구현 범위

- 횃불 기본 점화 상태를 꺼짐으로 변경
- 횃불 점화 상태에 따라 기존 Fire Niagara 활성/비활성 처리
- FirePit 점화 Actor C++ 구현
- `BP_SingijeonFirePit` 생성, FirePit 메시와 불 Niagara 적용
- `LV_Singijeon`에 FirePit 배치
- FirePit 점화 시 `Torch_Ignite / Trigger` 성공 보고
- 런타임 및 에셋 검증 추가

# 변경 예정 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Interaction/IgnitionSourceActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Interaction/FirePitActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/*
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonTorch.uasset
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonFirePit.uasset
Content/Maps/LV_Singijeon.umap
Scripts/ConfigureSingijeonFirePitFlow.py
Scripts/VerifySingijeonFirePitFlow.py
docs/Singijeon/specs/VR_INTERACTION.md
```

# 구현 단계

1. 횃불의 기본 꺼짐 상태와 VFX 동기화 구현
2. FirePit Overlap 점화와 Scenario 성공 보고 구현
3. Blueprint 생성, 메시/VFX 설정, 레벨 배치
4. 자동화 테스트, 에셋/DA/레벨 재조회

# 다른 Feature에 미치는 영향

- 변경은 `GF_Singijeon`의 횃불과 FirePit에 한정한다.
- Core Scenario에는 기존 `Torch_Ignite` 보고 계약만 사용한다.

# 검증 방법

- 새 횃불은 비활성 상태인지 확인
- FirePit 접촉 후 횃불만 활성화되는지 자동화 테스트
- FirePit BP 메시/VFX 및 레벨 배치 확인
- `INT_04 → INT_05 → INT_06` DA 흐름 재조회
- Win64 Development Editor 빌드와 전체 자동화 테스트

# 결과

- 횃불은 꺼진 상태로 시작하고 FirePit 접촉 후에만 점화된다.
- FirePit이 `Torch_Ignite`를 보고하며 점화된 횃불만 화차 Fuse를 시작할 수 있다.
- FirePit BP 생성, 메시/불 Niagara 설정, `LV_Singijeon` 배치를 완료했다.
- Editor 빌드, 전체 자동화 테스트 7개, 에셋/DA/레벨 재조회가 성공했다.
