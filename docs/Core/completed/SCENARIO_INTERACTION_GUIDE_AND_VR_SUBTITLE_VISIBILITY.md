# 작업

Scenario 전체에서 재사용하는 액터 상단 입력 가이드와 월드 메시 뒤에 가려지지 않는 VR 자막을 구현했다.

# 구현 내용

- `FScenarioInteraction`에 `GuideAction`과 `GuideText`를 추가했다.
- `Grab`, `Drag`, `Trigger`, `Press`, `Observe`, `Select`, `Speak`, `Combat`, `Interact`, `Hidden` 표시 정책을 제공한다.
- `UScenarioInteractionGuideComponent`가 현재 `TargetID + InteractionType`에 맞는 가장 가까운 Actor를 찾아 Bounds 위에 안내 Widget을 표시한다.
- Actor가 이동해도 위젯 위치와 HMD 방향을 갱신하며 Interaction 완료·실패·Skip 시 숨긴다.
- 모든 `AScenarioManagerActor`가 공통 Guide Component를 기본 소유하므로 Feature별 Manager 중복 구현이 필요 없다.
- 공심돈 관찰·보고·사격과 신기전 집기·장전·화차 이동·횃불·화로·도화선에 구체적인 가이드 문구를 기록했다.
- 자동 발사처럼 플레이어 입력이 없는 단계는 `Hidden`으로 설정했다.
- `BP_VRPlayerPawn.SubtitleHUD`를 OpenXR 호환 World Space로 유지하면서 HMD 전방 85cm로 가까이 배치하고 Translucency Sort Priority를 높였다.
- 공심돈처럼 9~18m 떨어진 관측 대상에서도 읽히도록 가이드가 카메라 거리에 비례해 확대되며 최대 크기를 제한한다.
- 가이드 컴포넌트가 늦게 초기화되더라도 현재 실행 중인 Interaction을 다시 조회해 표시한다.
- 활성 가이드 하나만 30Hz로 HMD 시점을 향해 Yaw를 갱신하며, Pitch/Roll은 고정해 VR에서 텍스트가 기울지 않게 했다.

# 변경 파일

- `Source/SuwonSiegeContestVR/Public/Core/Scenario/ScenarioTypes.h`
- `Source/SuwonSiegeContestVR/Public|Private/Core/Scenario/ScenarioInteractionGuide*`
- `Source/SuwonSiegeContestVR/Public|Private/Core/Scenario/ScenarioManagerActor.*`
- `Source/SuwonSiegeContestVR/Public|Private/Core/VR/VRPlayerPawn.*`
- `Content/Core/VR/Pawn/BP_VRPlayerPawn.uasset`
- `Content/Data/DA_Scenario_Singijeon.uasset`
- `Plugins/GameFeatures/GF_Gongsimdon/Content/Data/DA_Scenario_Gongsimdon.uasset`
- 관련 구성·검증 스크립트와 Scenario/Narration 명세

# 주요 결정 사항

- Core는 Feature 클래스를 참조하지 않고 기존 `ScenarioInteractableComponent` 계약으로 대상 Actor를 찾는다.
- 타입만으로 실제 입력을 알 수 없는 `Custom` 단계는 DA의 `GuideAction`으로 작성자가 명시한다.
- Screen Space WidgetComponent는 OpenXR HMD에서 누락될 수 있어 사용하지 않고, 카메라 부착 World Space를 가까이 배치한다.
- `docs/ARCHITECTURE.md`는 수정하지 않았다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development`: 성공
- Core Scenario 자동화 5개: 성공. 가이드 타입 해석과 Target Actor 표시·완료 숨김 포함
- 공심돈 3개 + 신기전 4개 자동화: 성공
- 공심돈·신기전 Scenario 에셋 가이드 및 BP 자막 설정 검증: 성공
- 전체 Blueprint 명령은 프로젝트와 무관한 기존 `/Game/NiagaraExamples/Utilities/Blueprints/EUB_SnapToActor`의 제거된 `SetMobility` 노드 때문에 실패했다. 작업 대상 `BP_VRPlayerPawn`은 개별 Compile 및 저장 성공

# 남은 문제

- Quest 3에서 위젯 실제 크기, 자막 양안 가독성, Actor별 Bounds 상단 높이를 확인하고 필요 시 Manager의 `HeightOffset`, `WorldScale` 또는 Interaction의 `GuideText`를 조정한다.
