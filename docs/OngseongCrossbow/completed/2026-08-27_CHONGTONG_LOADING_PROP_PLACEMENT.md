# 작업

VR 플레이어가 조작하는 총통의 화약, 쑤시개, 대포알 시작 위치를 `BP_PlayableChongtong` 뷰포트에서 조정할 수 있게 했다.

# 구현 내용

- `AChongtongCannonActor`에 `PowderSpawnPoint`, `RammerSpawnPoint`, `CannonballSpawnPoint` Scene Component를 추가했다.
- 세 컴포넌트는 `PlayerCameraAnchor`의 자식이며, `BP_PlayableChongtong` 뷰포트에서 직접 이동·회전할 수 있다.
- 기본 위치는 기존 런타임 배치와 동일하다: 화약 `(65, -38, -65)`, 쑤시개 `(65, 0, -65)`, 대포알 `(65, 38, -65)`.
- `SpawnPlaceholderLoadingItems`는 각 컴포넌트의 월드 Transform에서 해당 장전물을 생성한다.

# 변경 파일

- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/ChongtongCannonActor.h`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Ongseong/ChongtongCannonActor.cpp`

# 주요 결정 사항

장전물은 레벨에 미리 배치되는 액터가 아니라 `BP_PlayableChongtong`이 BeginPlay에 생성하고 내부 목록으로 추적한다. 따라서 레벨에 별도 액터를 놓는 방식 대신, 뷰포트에서 조정 가능한 Scene Component를 생성 기준점으로 사용했다. `PlayerCameraAnchor` 자체는 장전 완료 후 VR 플레이어 장착 시점에도 사용하므로 움직이지 않는다.

# 테스트 결과

- `git diff --check` 통과.
- `SuwonSiegeContestVREditor Win64 Development` UnrealBuildTool 실행 성공 (exit code 0).

# 남은 문제

- HMD에서 실제 손 도달 범위와 장전 판정 반경(기본 30cm)을 확인해 최종 컴포넌트 위치를 조정해야 한다.
