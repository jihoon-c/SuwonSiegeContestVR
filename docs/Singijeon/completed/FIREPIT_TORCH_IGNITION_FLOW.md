# 작업

FirePit 기반 횃불 점화 순서 구현

# 구현 내용

- `AIgnitionSourceActor`의 기본 상태를 꺼짐으로 변경했다.
- 횃불 점화 상태가 바뀌면 BP에 배치된 Niagara 효과도 함께 활성/비활성화한다.
- `AFirePitActor`를 추가해 꺼진 횃불 접촉 시에만 점화하고 `Torch_Ignite / Trigger`를 보고한다.
- `BP_SingijeonFirePit`에 `Geometric_Fire_Pit` 메시와 `NS_Fire`를 적용했다.
- `LV_Singijeon`에 `BP_SingijeonFirePit_Playable`을 배치했다.
- 기존 DA의 `INT_04 횃불 집기 → INT_05 FirePit 점화 → INT_06 화차 도화선` 순서를 실제 게임플레이 판정으로 강제했다.

# 변경 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Interaction/FirePitActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Interaction/IgnitionSourceActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonHwachaTests.cpp
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonTorch.uasset
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonFirePit.uasset
Content/Maps/LV_Singijeon.umap
Scripts/ConfigureSingijeonFirePitFlow.py
Scripts/VerifySingijeonFirePitFlow.py
docs/Singijeon/specs/VR_INTERACTION.md
```

# 주요 결정 사항

- Scenario 순서는 이미 올바르므로 DA Interaction을 추가하지 않고 누락됐던 실제 완료 보고자를 구현했다.
- Scenario Manager가 현재 `Torch_Ignite` 단계를 승인한 뒤에만 횃불 상태를 바꿔 조기 접촉으로 인한 순서 우회와 진행 정지를 막았다.
- 도화선은 기존 `IgnitionSourceInterface` 활성 상태 검사를 유지해 꺼진 횃불을 거부한다.
- 횃불 VFX는 특정 Niagara 이름에 의존하지 않고 Actor에 포함된 FX 컴포넌트와 점화 상태를 동기화한다.

# 테스트 결과

- Win64 Development Editor 빌드 성공
- 새 횃불 비활성, 꺼진 횃불 Fuse 거부, FirePit 점화, 점화 후 Fuse 허용 자동화 테스트 성공
- 전체 `SuwonSiegeContestVR` 자동화 테스트 7개 성공
- FirePit BP 메시/NS_Fire, 횃불 VFX, DA 순서, 레벨 배치 재조회 성공

# 남은 문제

- FirePit과 횃불 Niagara의 크기와 위치는 실제 Quest VR Preview에서 최종 시각 조정이 필요할 수 있다.
