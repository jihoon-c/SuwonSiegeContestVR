# 작업

VR Grab 판정을 시각 Primitive의 Pivot이 아니라 Bounds 기준으로 보완

# 구현 내용

- `VRGrab` 태그가 붙은 Primitive Component는 컨트롤러와 컴포넌트 중심점 거리가 아닌 렌더 Bounds까지의 최단거리로 선택한다.
- 길이가 긴 손잡이의 끝부분을 잡아도 중앙 Pivot 기준 15cm 제한 때문에 놓치는 문제를 해결한다.
- Primitive가 아닌 기존 Scene Component Grab 지점은 기존 중심점 거리 판정을 유지한다.

# 변경 파일

`Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`

# 주요 결정 사항

Grab 반경 자체를 전역으로 늘리지 않아 인접한 화살·횃불을 잘못 잡을 가능성을 높이지 않는다.

# 테스트 결과

신기전 화차의 보이는 좌·우 원통을 `VRGrab` 대상으로 지정했다. Editor Development 빌드와 `SuwonSiegeContestVR.GF_Singijeon` 자동화 테스트 4종을 통과했다.

# 남은 문제

Quest 3에서 실제 컨트롤러 Grip 위치와 원통 끝의 체감 Grab 범위를 확인한다.
