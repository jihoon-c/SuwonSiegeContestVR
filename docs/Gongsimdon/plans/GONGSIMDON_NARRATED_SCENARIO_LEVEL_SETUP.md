# 공심돈 나레이션 시나리오 레벨 구성 계획

**상태: 완료 (2026-08-22)**

## 목적

공심돈 플러그인의 27개 나레이션 음원과 기존 13개 Action Interaction을 하나의 명시적 Scenario 흐름으로 구성하고 `LV_Gongsimdon`에서 자동 실행되게 한다.

## 작업 전 상태

- `DA_Scenario_Gongsimdon`에는 나레이션 없이 Action Interaction 13개만 연결되어 있다.
- 플러그인 `Asset/Narration`에는 01~27 음원이 존재하지만 전용 Data Table이 없다.
- Scenario의 `NarrationTable`과 Level Scenario Manager의 나레이션 해석 결과가 비어 있다.
- 관찰, 적 등장·퇴각, 보고, 봉돈 확인, 사격 판정 Actor는 이미 Level에 배치되어 있다.

## 구현 범위

- 플러그인 전용 `DT_Narration_Gongsimdon` 생성 및 음원·자막 연결
- 27개 Narration과 기존 13개 Action을 스토리 순서대로 교차 연결
- Scenario Definition과 Level Scenario Manager/Experience 연결
- 기존 구성 스크립트가 나레이션 흐름을 다시 지우지 않도록 동기화
- 에셋/자동화 검증과 작업 문서 업데이트

## 변경 파일

- `GF_Gongsimdon` Data Asset, Data Table, Level
- 공심돈 Scenario 구성·검증 스크립트
- 공심돈 자동화 테스트와 문서

## 구현 단계

1. 음원 27개, Action ID 13개, Level Actor/Manager 상태 진단
2. 전용 DT 생성과 한 행 단위 Stop 흐름 설정
3. 나레이션/Action 전체 순서 작성 및 Level 연결
4. 빌드·자동화·에셋 검증

## 다른 Feature에 미치는 영향

공심돈 전용 데이터와 스크립트에 한정한다. Main의 공심돈 진입/복귀 체크포인트 계약은 유지하며 `docs/ARCHITECTURE.md`는 수정하지 않는다.

## 검증 방법

- DT 27행과 각 음원 Soft Reference 확인
- Scenario 40개 Interaction의 ID, Type, Next 연결 확인
- Level의 단일 Scenario Manager가 공심돈 Experience/Scenario/DT를 해석하는지 확인
- 기존 관찰·보고·적군 Target ID가 Scenario와 모두 일치하는지 확인
