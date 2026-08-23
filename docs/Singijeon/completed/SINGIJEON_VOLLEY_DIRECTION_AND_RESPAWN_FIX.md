# 작업

신기전 화살 발사 방향 및 발사 직후 재등장처럼 보이는 현상 수정

# 구현 내용

- 슬롯 `+X` 고정 발사 대신 메시의 화살촉 로컬 축을 기준으로 발사한다.
- 현재 신기전 화살의 화살촉 축인 로컬 `-X`를 기본값으로 정의했다.
- 중력에 따라 궤적이 변해도 화살촉이 현재 속도 방향을 향하도록 Post Physics에서 회전을 보정한다.
- ISM에서 변환된 발사체와 최초 물리 화살이 화차 본체에 즉시 충돌하지 않도록 한다.
- 같은 일제 발사에 포함된 화살끼리 이동 충돌을 무시하여 출발점 정지와 겹침을 방지한다.

# 변경 파일

- `SingijeonProjectileActor.h/.cpp`
- `SingijeonHwachaActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 주요 결정 사항

메시 자체를 재임포트하거나 Blueprint 회전을 임의 변경하지 않고, `ArrowTipLocalAxis`로 메시 방향 차이를 명시한다. 다른 화살 메시를 사용하면 해당 Blueprint 기본값만 변경할 수 있다.

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- 화살 속도와 화살촉 방향 일치 및 화차 충돌 무시 회귀 테스트 성공

# 남은 문제

Quest 3에서 실제 궤적과 시각 속도는 최종 확인이 필요하다.
