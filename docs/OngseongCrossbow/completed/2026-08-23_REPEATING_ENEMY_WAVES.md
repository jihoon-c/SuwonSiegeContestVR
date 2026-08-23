# 작업

옹성 180초 방어 중 검병 10명·궁병 10명·충차 1대 Wave 반복 구현.

# 구현 내용

- 방어 Manager가 보병 전멸과 충차 파괴를 함께 추적한다.
- 보병 20명과 충차 1대가 모두 격파된 3초 후 Wave Manager를 초기화하고 같은 구성을 다시 시작한다.
- 다음 Wave 시작 시 새 충차 1대를 생성한다.
- 현재 Wave 번호를 Blueprint에서 조회할 수 있다.
- 180초 성공, 성문 파괴 실패, 재시도와 종료 시 다음 Wave 예약을 취소한다.
- 기존 충차와 적/총통 교전 계약은 변경하지 않았다.

# 변경 파일

- `OngseongDefenseScenarioManager.h/.cpp`
- `OngseongDefenseTests.cpp`
- `docs/OngseongCrossbow/STATUS.md`
- `docs/OngseongCrossbow/ARCHITECTURE.md`

# 주요 결정 사항

Wave는 고정 시간에 중첩 생성하지 않고 현재 검병 10명·궁병 10명·충차 1대가
모두 격파된 뒤 다음 그룹을 시작한다. 보병은 최대 6명까지 활성화하고 나머지는
처치에 맞춰 순차 보충하여 Pool 상한과 VR 동시 개체 수 예산을 유지한다.

# 테스트 결과

- UHT: 성공
- 변경된 Runtime 및 Test C++ 컴파일: 성공
- `SuwonSiegeContestVR Win64 Development`: 성공, 최종 실행 파일 링크 확인
- Editor 타깃 최종 링크: 실행 중인 Editor의 플러그인 DLL 점유로 보류
- Automation Test 실행: 새 Editor DLL 로드가 필요해 Editor 재시작 후 실행 필요

# 남은 문제

- Editor 재시작 후 `SuwonSiegeContestVR.Ongseong.Defense.Contracts` 실행
- PIE에서 실제 180초 반복, 성공/실패 직전 예약 취소와 HMD 성능 확인
