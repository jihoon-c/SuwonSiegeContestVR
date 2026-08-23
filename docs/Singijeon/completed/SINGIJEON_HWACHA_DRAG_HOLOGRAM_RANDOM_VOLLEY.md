# 작업

화차 Grab 추종, 화차 형상 목표 홀로그램, 중복 장전 방지, 10초 랜덤 전탄 발사 구현

# 구현 내용

- 좌·우 손잡이에 native `VRGrab` 지점을 추가해 어느 손으로든 잡아 끌 수 있게 했다.
- 플레이어 이동 중에도 화차가 손의 월드 이동량을 지연 없이 따라간다.
- 화차 전체 메시와 `M_HwachaHologram`을 사용한 반투명 목표를 250cm 전방에 표시한다.
- 목표 55cm 안에 도착하면 성공 보고 후 홀로그램 Transform에 스냅하고 운반을 잠근다.
- Slot delegate 중복 바인딩, Auto Fill 재진입, `Hwacha_Load` 중복 성공 보고를 차단했다.
- 90발 전체를 무작위 순서로 한 발씩 발사하며 첫 발부터 마지막 발까지 10초가 걸린다.

# 변경 파일

- Core `VRPlayerPawn.h/.cpp`
- `TwoHandCarryComponent.h/.cpp`
- `SingijeonHwachaActor.h/.cpp`, `SingijeonHwachaTests.cpp`
- `BP_SingijeonHwacha`, `M_HwachaHologram`
- 구성/검증 Python 스크립트

# 주요 결정 사항

화차 Grab은 Blueprint 이벤트 그래프 대신 Core의 범용 `VRGrab` 태그/Actor Hook을 사용한다. 발사 간격은 고정값이 아니라 `VolleyDuration / (총 발수 - 1)`로 계산한다.

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- 화차 타깃 자동화 성공
- GF_Singijeon 전체 자동화 3/3 성공
- 저장된 BP/Material/Level 에셋 검증 성공

# 남은 문제

- Quest 3 VR Preview에서 손잡이 Grab 반경, 홀로그램 시인성, 투사체 90개 동시 수명에 따른 프레임 타임을 최종 점검한다.
