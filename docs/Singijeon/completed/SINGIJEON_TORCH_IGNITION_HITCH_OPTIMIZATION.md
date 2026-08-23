# 작업

FirePit에서 횃불 점화 시 Niagara 활성화 렉 완화

# 구현 내용

- 횃불 Niagara를 레벨 시작 시 렌더링 없이 1 Tick 사전 준비한다.
- 꺼진 상태에서는 Niagara 인스턴스를 제거하지 않고 Pause 및 렌더링 비활성 상태로 유지한다.
- 점화 시 `Activate(true)`로 리셋하지 않고 기존 인스턴스의 시뮬레이션과 렌더링만 재개한다.
- 횃불 효과에 Niagara scalability와 기본 1500cm 거리 컬링을 적용한다.
- 소화 후에도 인스턴스를 보존해 반복 점화 시 재할당을 피한다.

# 변경 파일

- `IgnitionSourceActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 주요 결정 사항

현재 불꽃 외형과 Scenario 판정은 유지하고, 접촉 프레임에 집중되던 Niagara 인스턴스 초기화 비용을 레벨 시작 단계로 이동했다.

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- 횃불 Niagara 사전 준비, 비점화 렌더링 차단, 점화 시 렌더링 재개 회귀 테스트 성공

# 남은 문제

Quest 3 실기기에서 점화 전후 프레임타임을 최종 확인할 수 있다.
