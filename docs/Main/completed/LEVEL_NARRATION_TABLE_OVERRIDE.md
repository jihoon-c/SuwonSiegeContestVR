# 작업

Scenario Manager 레벨별 Narration Data Table Override 추가

# 구현 내용

- `BP_ScenarioManager`에 편집 가능한 `Scenario | Narration > Level Narration Table`을 추가했다.
- 값이 있으면 해당 레벨 DT를 사용하고, 비어 있으면 Scenario Definition의 Narration Table을 사용한다.
- 최종 선택 결과는 기존 읽기 전용 `Resolved Configuration > Narration Table`에서 확인할 수 있다.

# 변경 파일

```text
Source/SuwonSiegeContestVR/Public/Core/Scenario/ScenarioManagerActor.h
Source/SuwonSiegeContestVR/Private/Core/Scenario/ScenarioManagerActor.cpp
Source/SuwonSiegeContestVR/Private/Tests/ScenarioFrameworkTests.cpp
docs/Main/specs/SCENARIO_SYSTEM.md
```

# 주요 결정 사항

- Scenario Definition의 기본값은 유지하고 Level Manager 값은 선택형 Override로만 사용한다.
- 기존 레벨은 새 값을 비워 둔 상태에서도 이전과 동일하게 동작한다.

# 테스트 결과

- Win64 Development Editor 빌드 성공
- 기본 DT Fallback과 레벨 Override 우선순위 자동화 테스트 성공
- `SuwonSiegeContestVR` 자동화 테스트 6개 전부 성공

# 남은 문제

- 없음
