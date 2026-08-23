# 작업

화차 손잡이 직접 Grab, 도화선 위치 안내, Scenario 단계 대기형 순차 발사 수정

# 구현 내용

- 보이는 좌·우 하이라이트 원통을 실제 `VRGrab` 대상으로 지정하고 Grab/Release를 `UTwoHandCarryComponent`에 전달했다.
- VR Pawn이 긴 Primitive의 Pivot이 아닌 전체 Bounds에서 Grab 거리를 계산하도록 보완했다.
- 화차 Body를 Movable로 유지하고 한 손 이동은 바닥 충돌에 막히지 않는 non-sweep 추종을 사용한다.
- 화차 뒤쪽 중앙 도화선에 발광 `FuseGuide`를 추가했다. 배치 완료 후 표시되고 점화·발사 시 숨는다.
- Fuse 완료 시 다음 나레이션 때문에 `Hwacha_Fire`가 아직 열리지 않아도 발사 요청을 보존하고, Fire 단계가 되면 자동 시작한다.
- 10초 동안 실제 화살과 ISM 화살을 무작위로 한 발씩 제거하며 장전 수를 90에서 0까지 갱신한다.

# 변경 파일

- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Public/Singijeon/SingijeonHwachaActor.h`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Singijeon/SingijeonHwachaActor.cpp`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonHwachaTests.cpp`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`
- `Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonHwacha.uasset`
- 관련 구성·검증 스크립트와 명세 문서

# 주요 결정 사항

- 전역 Grab 반경을 키우지 않고 Primitive Bounds만 사용해 인접한 화살·횃불 오선택 가능성을 줄였다.
- Fuse 완료와 Fire 시작 사이의 나레이션을 정상 흐름으로 보고 보류된 발사 요청으로 연결했다.
- 도화선 접촉 대상은 점화된 횃불의 상단 `IgnitionArea`이며 횃불 손잡이는 점화원이 아니다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공
- `SuwonSiegeContestVR.GF_Singijeon` 자동화 테스트 4/4 통과
- 화차 BP/레벨 Carry·Fuse 검증 Commandlet 통과
- Drag·Volley 에셋 검증 Commandlet 통과

# 남은 문제

Quest 3 실기에서 원통 끝 Grab 체감, 횃불 접촉 유지 시간, 90발 발사 중 프레임 타임을 최종 확인한다.
