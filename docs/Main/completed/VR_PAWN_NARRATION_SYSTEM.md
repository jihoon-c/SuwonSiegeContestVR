# 작업

프로젝트 전용 VR Player Pawn 기반과 Data Table 기반 나레이션·자막 시퀀스 시스템을 구현했다.

# 구현 내용

- `AVRPlayerPawn` 및 `BP_VRPlayerPawn` 생성
- Camera, 좌우 Grip/Aim Controller, Widget Interaction 구성
- 카메라 전방 World Space 자막 HUD와 후속 이벤트 Widget HUD 구성
- `FNarrationSequenceRow` 및 `DT_Narration` 생성
- 음성 Soft Reference 비동기 로드 및 Audio Finished 기반 자막 종료
- Auto / WaitForContinue / Stop 진행 정책
- 행 종료 이벤트 배열, 후속 Widget Class, Blueprint Delegate 제공
- 음성 미완성 시 `PreviewDuration` 기반 자막 프리뷰 지원

# 변경 파일

- `Source/SuwonSiegeContestVR/Public/Core/Narration/*`
- `Source/SuwonSiegeContestVR/Private/Core/Narration/*`
- `Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`
- `Content/Core/VR/Pawn/BP_VRPlayerPawn.uasset`
- `Content/Core/Experience/Definitions/DT_Narration.uasset`
- `Scripts/CreateCoreNarrationAssets.py`
- `docs/Main/specs/NARRATION_SYSTEM.md`

# 주요 결정 사항

- Core는 특정 Game Feature를 직접 참조하지 않고 이름 기반 이벤트를 Broadcast한다.
- Data Table 행은 배열 순서가 아닌 `NextRow`로 연결하여 분기와 재사용에 대비한다.
- 상호작용 Widget은 `WaitForContinue` 행에서 표시하고 완료 시 `DismissNarrationWidget(true)`로 진행한다.
- 음성 라이브러리는 Soft Reference로 관리해 전체 음성이 한 번에 메모리에 적재되지 않게 한다.
- 기존 템플릿 `BP_XRPawn`과 `BP_XRGameMode`는 수정하지 않았다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development`: 성공
- `SuwonSiegeContestVR Win64 Development`: 성공
- `CompileAllBlueprints`: 오류 0, 경고 0, 로드 실패 0
- `BP_VRPlayerPawn` 부모: `/Script/SuwonSiegeContestVR.VRPlayerPawn`
- `DT_Narration` Row Structure: `/Script/SuwonSiegeContestVR.NarrationSequenceRow`

# 남은 문제

- Grab, Teleport, Turn과 GameMode 연동은 후속 `VR_PAWN_LOCOMOTION_INPUT` 작업에서 완료했다.
- Android 스탠드얼론 실기기에서 HUD 거리, 글자 크기, 음성 메모리와 성능 검증이 필요하다.
