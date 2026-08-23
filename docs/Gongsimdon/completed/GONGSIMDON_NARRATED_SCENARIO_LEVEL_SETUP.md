# 공심돈 나레이션 시나리오 레벨 구성

**완료일**: 2026-08-22

## 결과

- 플러그인의 나레이션 음원 27개를 `/GF_Gongsimdon/Data/DT_Narration_Gongsimdon`에 자막과 함께 연결했다.
- `/GF_Gongsimdon/Data/DA_Scenario_Gongsimdon`을 나레이션 27개와 Action 13개가 교차하는 단일 40단계 흐름으로 구성했다.
- 각 DT 행은 `AdvanceMode=Stop`, `NextRow=None`으로 두어 DT가 독자적으로 다음 음원을 연속 재생하지 않게 했다.
- `LV_Gongsimdon`의 Scenario Manager는 별도 Scenario/DT를 중복 소유하지 않고 `DA_Experience_Gongsimdon → DA_Scenario_Gongsimdon → DT_Narration_Gongsimdon` 순으로 해석한다.
- 기존 관찰 타깃 5개, 보고 Actor, Director, 적군 그룹의 Target ID를 새 흐름과 검증했다.
- 공심돈 재구성 스크립트와 Main 왕복 구성 스크립트가 같은 시나리오 정의를 사용하도록 공통 데이터 모듈로 통합했다.

## 진행 계약

```text
Experience
  → Scenario Definition (전체 순서)
    → Narration Interaction → DT 한 행 재생 → 종료 시 다음 Interaction
    → Action Interaction → Level Actor 성공 보고 → 다음 Interaction
  → 마지막 NAR 27 완료
  → L_Main / MAIN_AFTER_GONGSIMDON 복귀
```

나레이션과 Action의 전체 순서는 `Scripts/GongsimdonNarratedScenarioData.py`의 `ORDERED_INTERACTION_IDS`가 구성·검증 스크립트의 단일 기준이다.

## 실제 작업 연결점

- 보고 단계: `Gongsimdon_Report.SubmitReport(East, 6~8)` 호출이 성공 조건이다.
- 사격 단계: 적군 Soldier에 표준 `ApplyDamage`가 들어오면 `COMBAT_RETREATING`이 완료된다.
- 동물/쇳소리·봉돈 연출: Director의 `OnCueRequested(InteractionID, TargetID)`에 실제 Sound/VFX를 연결한다.
- 관찰 단계: 배치된 Observation Target의 위치와 `RequiredViewTime`, `RequiredViewAngle`, `MaxDistance`를 현장 기준으로 조정한다.

## 검증

- Unreal Python 에셋 검증: DT 27행, Scenario 40단계 순서·링크, Experience/Manager 해석, Level Actor와 Target ID 확인
- C++ 자동화: 실제 Scenario/DT 로드, `ValidateScenario`, 27개 나레이션 행과 Stop 계약 확인

## 남은 현장 작업

- 보고 UI/음성 입력과 실제 무기 입력 연결
- 동물/쇳소리, 봉돈 불·연기 Cue 연출 연결
- Quest 3에서 관찰 위치, 음량, 자막 타이밍 및 Main 왕복 동선 검증
