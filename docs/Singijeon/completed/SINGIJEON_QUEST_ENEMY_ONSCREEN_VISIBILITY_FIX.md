# 작업

Quest/Oculus OpenXR 실행 시 `LV_Singijeon`의 배경 적군 42명이 렌더링되지 않는 문제를 실제 VR 렌더 캡처 기준으로 수정했다.

# 구현 내용

- 수정 전 비교 캡처에서 PC 모노는 45명, OpenXR은 완전한 Actor 3명만 표시되는 것을 확인했다.
- Wave 액터에 동적으로 부착하던 42개 `USkeletalMeshComponent`를 Wave가 소유하는 경량 `ASkeletalMeshActor`로 전환했다.
- 각 프록시 Actor는 충돌, Actor Tick, 그림자를 비활성화했다.
- 8개 리더만 달리기 포즈를 평가하고 나머지는 Leader Pose를 공유해 VR 비용을 제한했다.
- 전경 3명과 포즈 리더는 화면 가시성 판정과 무관하게 포즈를 갱신하도록 설정했다.
- Editor Target을 `-game`으로 실행할 때 에디터 전용 화살 미리보기 컴포넌트가 null인 경우의 시작 크래시도 방어했다.

# 변경 파일

- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Public/Enemy/SingijeonEnemyWaveActor.h`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Enemy/SingijeonEnemyWaveActor.cpp`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonEnemyWaveTests.cpp`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Singijeon/SingijeonHwachaActor.cpp`
- `Scripts/InspectSingijeonEnemyVisibility.py`

# 주요 결정 사항

등록 여부나 Visible 플래그만 확인하는 테스트는 VR 렌더 실패를 검출하지 못한다. 배경 적군은 OpenXR Scene Proxy 수명주기가 보장되는 독립 Actor로 유지하고, PC 모노와 OpenXR 실제 프레임을 함께 비교한다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development`: 성공
- `SuwonSiegeContestVR.GF_Singijeon`: 7/7 성공
- 레벨 계측: Quest-safe 프록시 Actor 42개, Visible=true, HiddenInGame=false
- 수정 전 Oculus OpenXR 캡처: 전경 3명만 표시
- 수정 후 Oculus OpenXR 캡처: 전경 3명과 배경 42명, 총 45명 표시

# 남은 문제

없음. 에디터 재실행 후 VR Preview에서 최종 사용자 시야와 배치 감각만 확인하면 된다.
