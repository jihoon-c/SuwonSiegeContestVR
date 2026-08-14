**상태**: 완료 (2026-08-14)

# 목적

VR에서 잡은 신기전을 화차 슬롯에 넣을 때 발생하는 접근 위반 크래시를 제거한다.

# 현재 상태

- 크래시 스택은 `ASingijeonProjectileActor::PrepareForLoading_Implementation`에서 시작한다.
- Blueprint `TryRelease`를 `ProcessEvent(..., nullptr)`로 호출해 반환값을 포함한 함수 파라미터 메모리가 제공되지 않는다.
- Release 도중 충돌 상태가 바뀌면 슬롯 Overlap 장전 경로가 재진입할 가능성도 있다.

# 구현 범위

- Blueprint 함수 호출용 파라미터 프레임 생성
- 슬롯 장전 재진입 방지
- Release 이후 후보 액터 유효성 및 Attach 성공 여부 확인

# 변경 예정 파일

- `SingijeonProjectileActor.cpp`
- `SingijeonAmmoSlotComponent.cpp/.h`
- 관련 Singijeon 문서

# 검증 방법

- GF_Singijeon Editor 모듈 빌드
- 크래시 경로 정적 검증 및 가능하면 자동화 테스트
- VR Preview에서 Grab → 슬롯 Overlap 재확인 필요
