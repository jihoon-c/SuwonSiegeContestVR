# 작업

Quest 3에서 `BP_VRPlayerPawn`의 좌우 손이 보이도록 시각 메시를 추가했다.

# 구현 내용

- `LeftGrip`과 `RightGrip` Motion Controller 아래에 각각 Skeletal Mesh Component를 추가했다.
- 기존 XR 템플릿의 `SKM_MannyXR_left`, `SKM_MannyXR_right` 메시를 재사용했다.
- 기존 `ABP_MannequinsXR` 애니메이션 Blueprint를 양손에 연결했다.
- 기존 `BP_XRPawn`에서 읽은 좌우 손 Relative Transform을 동일하게 적용했다.
- 손 메시는 시각 표현 전용이며 충돌, 오버랩, 그림자는 비활성화했다.
- Quest 패키지에서도 Grab 입력이 확실히 등록되도록 `IMC_Default`를 런타임에 명시적으로 추가한다.
- Grab 포인트 탐색 반경을 6cm에서 15cm로 넓혔다.

# 변경 파일

- `Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`

# 주요 결정 사항

- Quest 전용 플러그인 종속성을 추가하지 않고 기존 OpenXR Grip 추적을 사용한다.
- 손 모양과 기존 애니메이션 자산을 재사용해 `BP_XRPawn`과 시각 구성을 맞춘다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공.
- `CompileAllBlueprints` 결과 오류 0, 경고 0, 로드 실패 0.
- Quest 3 실기 추적은 Editor 재시작 및 Android 재패키징 후 확인한다.

# 잔여 문제

- Grab Press/Release의 `PoseAlphaGrasp` 연결은 `VR_PAWN_SMOOTH_LOCOMOTION.md` 작업에서 완료했다.
- Index Curl, Point, Thumb Up의 개별 손가락 입력은 아직 연결하지 않았다.
