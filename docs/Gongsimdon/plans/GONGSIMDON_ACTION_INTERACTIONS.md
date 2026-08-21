# 목적

공심돈 스토리의 나레이션을 제외하고 관찰, 방향 확인, 적 식별, 보고, 봉돈 확인, 사격, 최종 경계 Interaction을 실제 판정 가능한 형태로 구현한다.

**상태: 완료 (2026-08-19)**

# 현재 상태

- `DA_Scenario_Gongsimdon`은 Intro 자동 완료와 5초 Wait만 있는 기초 자리표시자다.
- Core에 `UScenarioObservationComponent`, `UScenarioInteractableComponent`, `UScenarioManagerComponent`가 있다.
- `GF_Gongsimdon`은 Content-only 플러그인이며 전용 런타임 클래스가 없다.

# 구현 범위

- `GF_Gongsimdon` 런타임 모듈 추가
- Scenario 요청을 Level Actor에 연결하는 공심돈 Interaction Director 구현
- 재사용 가능한 관찰 타깃 Actor 구현
- 적 위치/규모 보고 판정 Actor 구현
- 사격 피격 및 필요 명중 수 판정 Actor 구현
- 나레이션을 제외한 공심돈 Scenario Interaction 배열 구성
- 공심돈 Level에 Director와 판정 Actor 배치

# 변경 예정 파일

```text
Plugins/GameFeatures/GF_Gongsimdon/GF_Gongsimdon.uplugin
Plugins/GameFeatures/GF_Gongsimdon/Source/GF_Gongsimdon/*
Plugins/GameFeatures/GF_Gongsimdon/Content/Data/DA_Scenario_Gongsimdon.uasset
Plugins/GameFeatures/GF_Gongsimdon/Content/Maps/LV_Gongsimdon.umap
Scripts/ConfigureGongsimdonActionInteractions.py
Scripts/VerifyGongsimdonActionInteractions.py
docs/Gongsimdon/STATUS.md
```

# 구현 단계

1. Feature 런타임 모듈과 Interaction Actor 클래스 추가
2. 자동화 테스트와 Editor 빌드
3. Scenario Action Interaction 구성
4. Level Actor 배치 및 참조 검증
5. 문서 완료 기록

# 다른 Feature에 미치는 영향

- Core 클래스는 변경하지 않는다.
- 공심돈 모듈은 Core Scenario API만 의존한다.
- 무기/적 시각 자산은 후속 Blueprint에서 판정 Actor에 연결할 수 있다.

# 검증 방법

- Win64 Development Editor 빌드
- 보고 정답/오답과 사격 명중 수 자동화 테스트
- Scenario 배열에 Narration 타입이 없는지 검증
- Level의 Director, Observation Target, Report, Combat Actor 재조회
- 전체 `SuwonSiegeContestVR` 자동화 테스트

# 구현 결과

- `GF_Gongsimdon` 런타임 모듈과 Feature 전용 Interaction 클래스 추가
- 나레이션 없는 Action Interaction 13단계 구성
- Level에 Director 1, 정적 Observation Target 5, 이동 Enemy Group 1, Report Actor 1 배치
- 보고 정답과 사격 완료 흐름 자동화 테스트 성공
- Win64 Development Editor 빌드 및 전체 자동화 8/8 성공
