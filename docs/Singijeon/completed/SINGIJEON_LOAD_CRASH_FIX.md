# 작업

VR에서 잡은 신기전을 화차 슬롯에 넣을 때 발생하던 접근 위반 크래시 수정

# 구현 내용

- `TryRelease` Blueprint 함수 호출 시 `FStructOnScope`로 정상 파라미터 프레임 제공
- Release가 충돌 상태를 변경하는 동안 슬롯 Overlap이 재진입하지 않도록 장전 가드 추가
- Release 도중 탄약 액터가 파괴되면 장전 중단
- 슬롯 부착 전에 물리 시뮬레이션과 속도를 정리해 Attach 실패 방지
- Attach 실패 시 `LoadedAmmunition` 상태 복구

# 변경 파일

- `SingijeonProjectileActor.cpp`
- `SingijeonAmmoSlotComponent.cpp`
- `SingijeonAmmoSlotComponent.h`

# 주요 결정 사항

기존 Blueprint Grab 컴포넌트와의 느슨한 연결은 유지하되, `ProcessEvent` 호출 규약과 장전 트랜잭션의 유효성 검사를 보강했다.

# 테스트 결과

- 크래시 덤프에서 `PrepareForLoading_Implementation`의 null 파라미터 프레임 호출 확인
- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공
- GF_Singijeon DLL 링크 성공

# 남은 문제

- Quest Link VR Preview에서 Grab → 슬롯 삽입 체감 및 스냅 방향을 최종 확인해야 한다.
