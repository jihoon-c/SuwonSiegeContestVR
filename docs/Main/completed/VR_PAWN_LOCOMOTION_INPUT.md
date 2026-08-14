# 작업

`BP_XRPawn`의 Grab, Teleport, Snap Turn 입력을 `BP_VRPlayerPawn`에 이식하고 `LV_Singijeon`의 기본 Pawn으로 연결했다.

# 구현 내용

- Enhanced Input의 `IA_Move`, `IA_Turn`, 좌우 Grab Press/Release를 Core Pawn에 바인딩
- 반경 6cm 내 가장 가까운 `BP_GrabComponent` 탐색 및 기존 `TryGrab` / `TryRelease` 호출
- 투사체 경로 충돌과 NavMesh 투영 기반 Teleport
- 기존 `BP_TeleportVisualizer` 목적지 표시 재사용
- HMD 위치를 회전 피벗으로 유지하는 45도 Snap Turn
- `LV_Singijeon` World Settings에 `BP_XRGameMode` 명시
- `BP_XRGameMode.DefaultPawnClass = BP_VRPlayerPawn` 확인

# 변경 파일

- `Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`
- `Source/SuwonSiegeContestVR/SuwonSiegeContestVR.Build.cs`
- `Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap`
- `Scripts/ApplyVRPawnToSingijeon.py`
- 관련 Main / Singijeon 문서

# 주요 결정 사항

- 템플릿 Pawn이나 Grab Component를 복제하지 않고 기존 `BP_GrabComponent` 계약을 재사용했다.
- Core는 Singijeon을 참조하지 않으며 Singijeon Level이 Core Pawn GameMode를 선택하는 의존 방향만 사용한다.
- 작업 전부터 수정되어 있던 `BP_XRGameMode`는 저장하거나 덮어쓰지 않고 현재 Default Pawn 설정만 검증했다.
- 다른 Level에는 Map Override를 추가하지 않았다.

# 테스트 결과

- `SuwonSiegeContestVR Win64 Development`: 컴파일 및 링크 성공
- `SuwonSiegeContestVREditor Win64 Development`: 실행 중 Editor를 유지한 채 `ModuleWithSuffix` 방식으로 컴파일 및 링크 성공
- `CompileAllBlueprints`: Blueprint 오류 0, 경고 0, 로드 실패 0. 명령let 프로세스는 기존 Editor와의 MCP 포트 충돌 및 샌드박스 DDC 접근 오류 때문에 종료 코드 1을 반환함
- `LV_Singijeon` 저장 후 GameMode: `/Game/XRFramework/Blueprints/BP_XRGameMode.BP_XRGameMode_C`
- GameMode Default Pawn: `/Game/Core/VR/Pawn/BP_VRPlayerPawn.BP_VRPlayerPawn_C`

# 남은 문제

- 실행 중인 Unreal Editor는 기존 모듈을 로드하고 있으므로 새 입력 구현을 반영하려면 Editor 재시작이 필요하다.
- OpenXR 런타임이 없는 자동화 환경이라 Grab / Teleport / Turn 실기 입력은 검증하지 못했다.
- Teleport 사용을 위해 `LV_Singijeon`에 유효한 NavMesh 영역이 배치되어 있어야 한다.
