# 신기전 런타임 적군 화면 표시 원인 분석 및 수정

상태: 완료

# 목적

`LV_Singijeon` 게임 시작 시 적군이 화면에 전혀 보이지 않는 문제를 실제 게임 월드와 플레이어 시야 기준으로 해결한다.

# 현재 상태

- 기존 테스트는 동적 Skeletal Mesh Component의 등록·Visible·Hidden 플래그만 확인했다.
- 실제 플레이어 시작점, SpawnVolume, 최종 적군 Bounds가 같은 시야/공간에 있는지는 검증하지 않았다.
- PC 모노 실제 렌더 캡처에서는 45명이 표시됐다.
- 같은 빌드의 Quest/Oculus OpenXR 렌더 캡처에서는 완전한 `EnemySoldierActor` 3명만 표시되고, Wave 액터에 런타임 부착한 42개 Skeletal Mesh Component가 모두 누락됐다.
- 원인은 등록/Visible 플래그가 아니라 OpenXR 렌더 경로에서 동적 부착 Component의 Scene Proxy 수명주기가 보장되지 않은 구현 방식이었다.

# 구현 범위

- 게임 월드 BeginPlay 이후 적군 위치·스케일·Bounds·렌더 상태를 계측한다.
- 플레이어 시작점과 적군 편대의 실제 공간 관계를 검증한다.
- 확정된 원인만 `GF_Singijeon` Enemy Wave에 최소 수정한다.
- 레벨 기반 런타임 테스트로 회귀를 막는다.

# 변경 예정 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- 필요 시 `LV_Singijeon` Enemy Wave 배치 설정
- 신기전 진단/검증 스크립트와 완료 문서

# 구현 단계

1. 저장된 SpawnVolume·PlayerStart·Hwacha·적군 Proxy 위치와 Bounds를 계측한다. (완료)
2. PIE/게임 월드 BeginPlay 직후 실제 생성 상태를 확인한다. (완료)
3. 42개 배경 적군을 Wave 소유 동적 Component에서 경량 `ASkeletalMeshActor`로 전환한다. (완료)
4. 8개 포즈 리더만 애니메이션을 평가하고 나머지는 Leader Pose를 공유한다. (완료)
5. Editor 빌드, 자동화 테스트, PC/Quest 실제 렌더 캡처를 실행한다. (완료)

# 다른 Feature에 미치는 영향

`GF_Singijeon` Enemy Wave에만 적용한다. Core, Shared 적군, 다른 Game Feature는 수정하지 않는다.

# 검증 방법

- `SuwonSiegeContestVREditor Win64 Development` 빌드
- `SuwonSiegeContestVR.GF_Singijeon` 자동화 테스트
- `LV_Singijeon` 게임 월드 BeginPlay 후 적군 수·위치·Bounds·렌더 가능 상태 확인
- PC 모노와 Oculus OpenXR 각각의 실제 화면 캡처에서 45명 편대 표시 비교
