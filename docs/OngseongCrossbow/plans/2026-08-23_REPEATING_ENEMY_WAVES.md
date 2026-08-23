> **2026-08-24 대체 예정**: 반복 Wave 방식은 `plans/2026-08-24_SUSTAINED_ENEMY_POPULATION.md`의
> 적 상시 유지 스폰으로 대체된다. 이 문서는 기존 구현의 배경 기록으로만 유지한다.

# 목적

옹성 방어가 진행되는 180초 동안 검병 10명·궁병 10명·충차 1대로 구성된 적 Wave를 반복한다.

# 현재 상태

`AOngseongEnemyWaveManager`는 한 Wave의 스폰, 전멸, 퇴각과 Pool 반환을 지원하지만
기본 5명을 모두 스폰한 뒤 종료된다. `AOngseongDefenseScenarioManager`는 180초 방어
타이머를 소유하지만 전멸 이벤트를 다음 Wave 시작으로 연결하지 않는다.

# 구현 범위

- 보병 20명과 충차 1대가 모두 격파되면 설정 가능한 대기 시간 후 다음 Wave 시작
- 180초 성공, 성문 파괴 실패, 재시도 시 예약된 Wave 취소
- Wave 번호를 Blueprint/HUD 확장에서 조회할 수 있도록 런타임 상태 제공
- 기존 충차, 궁병 총통 우선 표적, 검병 성문 공격 계약 유지

# 변경 예정 파일

- `OngseongDefenseScenarioManager.h/.cpp`
- `OngseongDefenseTests.cpp`
- `docs/OngseongCrossbow/STATUS.md`
- `docs/OngseongCrossbow/ARCHITECTURE.md`

# 구현 단계

1. 방어 Manager가 `OnAllEnemiesDefeated`를 구독한다.
2. 보병 전멸과 충차 파괴를 각각 추적하고 둘 다 충족되면 다음 Wave 타이머를 예약한다.
3. 타이머 만료 시 Wave Manager를 Reset한 뒤 동일 구성을 다시 시작한다.
4. 성공·실패·재시도·종료 경로에서 다음 Wave 타이머를 정리한다.
5. 기본 설정과 상태 계약을 Automation Test에 추가한다.

# 다른 Feature에 미치는 영향

변경은 `GF_OngseongCrossbow` 내부 시나리오 Manager에 한정한다. Shared Gameplay와
다른 Game Feature의 적/전투 계약은 변경하지 않는다.

# 검증 방법

- `SuwonSiegeContestVREditor Win64 Development` 빌드
- `SuwonSiegeContestVR.Ongseong` Automation Test
- PIE에서 Wave 전멸 후 다음 Wave 시작, 180초 성공 후 추가 스폰 없음 확인

# 진행 상태

구현과 Game 타깃 빌드는 완료했다. Editor 타깃은 UHT 및 변경 소스 컴파일까지
통과했으나 실행 중인 Editor가 플러그인 DLL을 점유해 최종 링크와 Automation Test
실행은 Editor 재시작 후 확인해야 한다.
