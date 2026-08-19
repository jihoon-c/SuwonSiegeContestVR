**상태**: 완료 (2026-08-13)

# 목적

기존 `BP_XRPawn`의 Grab, Teleport, Snap Turn 입력 동작을 프로젝트 전용 `BP_VRPlayerPawn`으로 이식하고, `LV_Singijeon`에서 새 Pawn을 기본 플레이어 Pawn으로 사용한다.

# 현재 상태

- `BP_VRPlayerPawn`은 `AVRPlayerPawn` 기반이며 Camera, Motion Controller, Widget Interaction, 나레이션 HUD를 가진다.
- Grab, Teleport, Turn 입력 처리는 아직 템플릿 `BP_XRPawn`에만 있다.
- `LV_Singijeon`은 프로젝트 전역 `BP_XRGameMode`를 사용하며 별도 GameMode Override가 없다.
- 기존 작업 트리의 `BP_XRGameMode`에는 사용자 변경이 있으므로 이번 작업에서 덮어쓰지 않는다.

# 구현 범위

- `AVRPlayerPawn`에 Enhanced Input 바인딩 추가
- 기존 `BP_GrabComponent`의 `TryGrab` / `TryRelease` 계약 재사용
- NavMesh 투영 기반 Teleport와 기존 `BP_TeleportVisualizer` 재사용
- HMD 위치를 회전 중심으로 유지하는 Snap Turn 추가
- 기존 `BP_XRGameMode`의 `DefaultPawnClass = BP_VRPlayerPawn` 설정을 보존하고 `LV_Singijeon`의 World Settings에만 명시적으로 지정

# 변경 예정 파일

- `Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`
- `Source/SuwonSiegeContestVR/SuwonSiegeContestVR.Build.cs`
- `Content/XRFramework/Blueprints/BP_XRGameMode.uasset` (기존 설정 확인만 수행, 이번 작업에서 저장하지 않음)
- `Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap`
- 관련 Main / Singijeon 문서

# 구현 단계

1. 원본 Pawn과 Grab Component의 함수·변수·기본값을 리플렉션으로 확인한다.
2. Core Pawn에 입력 바인딩 및 Grab / Teleport / Snap Turn을 구현한다.
3. C++ Editor 타깃을 빌드한다.
4. 기존 `BP_XRGameMode`의 `DefaultPawnClass`가 `BP_VRPlayerPawn`인지 확인한다.
5. `LV_Singijeon` World Settings에 `BP_XRGameMode`를 지정한다.
6. Blueprint compile, 레벨 로드 및 설정을 검증한다.

# 다른 Feature에 미치는 영향

- Core Pawn 기능이므로 다른 Feature에서도 `BP_VRPlayerPawn`을 선택하면 같은 입력을 재사용할 수 있다.
- 프로젝트 전역 GameMode와 다른 Level은 변경하지 않는다.
- Core에서 `GF_Singijeon`을 참조하지 않는다. Singijeon Level이 Core GameMode를 참조하는 허용 방향만 추가된다.

# 검증 방법

- `SuwonSiegeContestVREditor Win64 Development` 빌드
- `BP_VRPlayerPawn` 및 신규 GameMode Blueprint compile
- `LV_Singijeon` 로드 및 World Settings / Default Pawn 클래스 확인
- `CompileAllBlueprints` 오류 확인
- OpenXR 런타임이 있는 환경에서 Grab / Teleport / Turn 실기 동작은 별도 확인
