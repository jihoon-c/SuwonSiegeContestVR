# 목적

신기전 발사 후 적군이 잠시 혼란에 빠진 뒤 생존 병력이 후퇴하도록 만들고, Enemy Wave Actor를 복제해 병력을 늘려도 VR 성능 예산과 이벤트 연결이 안전하게 유지되도록 한다.

상태: 완료 (2026-08-26)

# 현재 상태

- 발사 후 `Panicking`은 3초간 동작한다.
- 기본 피격률이 1.0이라 전원이 제거되며, 생존자가 있으면 후퇴하지 않고 다시 전진한다.
- Wave 복제본도 같은 화차 이벤트를 받을 수 있으나 설정 스크립트가 복제본을 삭제한다.
- Wave 수만큼 전경 Actor와 애니메이션 Pose Leader 수가 그대로 늘어난다.

# 구현 범위

- `Panicking -> Retreating -> Retreated` 상태 전이 추가
- 기본 사상률을 낮추고 생존자 후퇴 거리/시간/종료 숨김을 에디터 설정으로 노출
- 다중 Wave에서 Interactive Actor와 Pose Leader 예산 자동 분배
- Wave 위치를 Seed에 혼합해 복제본 편대 배치 반복 완화
- 비활성/후퇴 완료 메시의 Animation Tick 중지
- 설정/검증 스크립트가 기존 Wave를 삭제하거나 Transform을 변경하지 않도록 수정

# 변경 예정 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- `Scripts/ConfigureSingijeonEnemyWave.py`
- `Scripts/VerifySingijeonEnemyWave.py`
- `Scripts/VerifySingijeonSamuraiEnemyWave.py`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 구현 단계

1. 후퇴 상태와 설정값 구현
2. 다중 Wave 런타임 예산 산정 구현
3. 숨김 상태 Animation Tick 절감
4. 자동화 테스트와 에디터 검증 스크립트 보강
5. 빌드/테스트 후 완료 문서 기록

# 다른 Feature에 미치는 영향

`GF_Singijeon` 내부 구현만 변경한다. Shared Enemy Actor와 Core에는 새 의존성을 추가하지 않는다. 레벨에 배치된 Actor Transform은 변경하지 않는다.

# 검증 방법

- C++ 자동화 테스트로 혼란, 사상자 처리, 후퇴, 종료 숨김을 확인
- 두 Wave를 생성해 총 병력 증가와 런타임 예산 분배를 확인
- 프로젝트 빌드 및 `GF_Singijeon.EnemyWave` 테스트 실행
- 레벨 검증 스크립트로 모든 배치 Wave의 설정과 참조 확인

# 검증 결과

- `SuwonSiegeContestVREditor Win64 Development`: 성공
- `GF_Singijeon.EnemyWave` 자동화 3건: 성공
- Enemy Wave 레벨 설정 검증: 성공
- Samurai LOD/Pose 공유 경로 검증: 성공
