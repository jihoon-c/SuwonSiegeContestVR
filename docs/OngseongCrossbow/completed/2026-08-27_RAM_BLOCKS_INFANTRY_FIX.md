# 작업

적 병사가 충차(Ram)에 가로막혀 전진하지 못하는 문제 수정.

# 원인

`AOngseongRamActor::RamMesh` 가 StaticMeshComponent 기본 충돌 설정(Pawn 차단)을 그대로 사용했다.
호위 보병은 충차와 같은 통로를 따라 전진하고, 충차 자체는 `SetActorLocation(..., /*bSweep=*/false)`
로 스크립트 이동하므로 병사를 밀어내지도 않는다. 결과적으로 CharacterMovement 의 sweep 이 충차
메시에 막혀 병사들이 그 자리에 끼인다.

# 구현 내용

`RamMesh` 충돌 설정을 생성자에서 명시:

* `ECollisionEnabled::QueryOnly`, ObjectType `WorldDynamic`
* 모든 채널 Block 유지 → 총통 포탄/쇠뇌 볼트/시야 트레이스는 여전히 충차에 명중
* `ECC_Pawn` 만 `ECR_Ignore` → 병사는 통과
* `SetCanEverAffectNavigation(false)` → 움직이는 메시가 navmesh 를 매 프레임 깎아 호위가
  재경로 계산하는 것을 방지

# 변경 파일

* `Plugins/.../Private/Ongseong/OngseongRamActor.cpp`

# 주요 결정 사항

* 충차를 병사가 밀 수 있게 하거나 회피 반경을 넣는 대신, Pawn 채널만 무시하도록 했다.
  충차 이동은 스크립트 경로이므로 물리 상호작용이 필요 없다.

# 테스트 결과

* 코드 수정만 완료. 인게임 전진 검증은 아직 수행하지 않음.

# 남은 문제

* `BP_OngseongRam` 에 충돌 프로파일 오버라이드는 없는 것으로 확인했으나, 에디터에서 RamMesh 의
  Collision Preset 이 수동 지정되어 있다면 C++ 기본값을 덮어쓰므로 확인 필요.
* 게이트(`BP_OngseongGate`) 앞에서의 병목은 이번 수정 범위가 아니다.
