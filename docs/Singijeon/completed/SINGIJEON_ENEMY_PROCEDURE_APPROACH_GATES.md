# 작업

신기전 발사 절차에 따른 적군 접근 거리 제한

# 구현 내용

- 기존 공용 Route와 적군별 편대 최종 위치를 유지했다.
- 첫 장전 35%, 화차 배치 60%, 도화선 점화 82%, 발사 중 95%로 접근 상한을 적용했다.
- 상한 도착 시 Enemy Wave Tick을 끄고 다음 화차 절차 이벤트에서 다시 이동시킨다.
- 전탄 발사 완료 후 생존자가 있을 때만 최종 도착점까지 이동을 개방한다.
- 절차가 취소되어도 이미 전진한 적군은 뒤로 순간이동하지 않는다.
- 모든 접근 비율과 기능 사용 여부를 Level Actor에서 수정할 수 있다.

# 변경 파일

- `SingijeonHwachaActor.h/.cpp`
- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- `ConfigureSingijeonEnemyWave.py`, `VerifySingijeonEnemyWave.py`
- `LV_Singijeon`

# 주요 결정 사항

Scenario Manager를 직접 참조하지 않고 화차가 소유한 장전·배치·점화·발사 이벤트를 사용했다. 따라서 Scenario Data 구성이 바뀌어도 실제 플레이 절차 결과를 기준으로 이동한다.

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- Level 접근 게이트 설정 검증 성공
- GF_Singijeon 전체 자동화 3/3 성공
- 단계별 정지·재개 및 최종 도착 개방 테스트 성공

# 남은 문제

- Quest 3 플레이에서 지형 길이에 따른 35/60/82/95% 체감 위치를 최종 조정할 수 있다.
