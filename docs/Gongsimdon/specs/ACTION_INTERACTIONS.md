# 공심돈 Action Interaction

`DA_Scenario_Gongsimdon`은 현재 나레이션을 포함하지 않고 아래 동작만 순차 실행한다.

```text
SCAN_PERIMETER
  → SOUND_ANIMAL → CHECK_ANIMAL
  → SOUND_METAL → CHECK_METAL
  → REVEAL_ENEMY → IDENTIFY_ENEMY
  → REPORT_ENEMY
  → ENABLE_BEACON → CHECK_BEACON
  → RETREAT_ENEMY → SHOOT_RETREATING
  → FINAL_SCAN
```

## Level Actor 역할

| 클래스 | 역할 |
|---|---|
| `AGongsimdonScenarioDirectorActor` | 현재 Scenario 요청을 Target ID가 일치하는 Level Actor에 전달 |
| `AGongsimdonObservationTargetActor` | HMD 시야각, 거리, 응시 시간으로 Observe 완료 |
| `AGongsimdonReportActor` | 방향과 적 규모를 검증하고 Custom 완료 |
| `AGongsimdonEnemyGroupActor` | 적군 7명 Spawn, 이동 관찰 중심, 퇴각 피격 Combat 완료 |

## 연출 연결

Director의 `OnCueRequested(InteractionID, TargetID)`에 다음 연출을 연결한다.

| Target ID | 연출 |
|---|---|
| `CUE_ANIMAL_SOUND` | 첫 번째 방향에서 동물/수풀 소리 재생 |
| `CUE_METAL_SOUND` | 다른 방향에서 쇳소리 재생 |
| `CUE_REVEAL_ENEMY` | Enemy Group 표시 및 접근 시작 |
| `CUE_ENABLE_BEACON` | 봉돈 불/연기 활성화 |
| `CUE_RETREAT_ENEMY` | Enemy Group 퇴각 Spline 이동 시작 |

위 Sequence는 현재 요청 이벤트 발생 직후 자동 완료된다. 연출 종료까지 기다려야 한다면 `bCompleteOnStart`를 끄고 연출 완료 시 Scenario Manager의 `ReportInteractionResult`를 호출한다.

## 보고 연결

보고 UI 또는 음성 인식 결과에서 배치된 `Gongsimdon_Report` Actor의 다음 함수를 호출한다.

```text
SubmitReport(Direction, EnemyCount)
```

기본 정답은 `East`, 6~8명이다. 오답은 Scenario를 실패시키지 않고 현재 보고 단계를 유지한다.

## 사격 연결

배치된 `Gongsimdon_EnemyGroup`이 Spawn한 개별 `AEnemySoldierActor`에 투사체/무기가 표준 `ApplyDamage`를 호출한다. 기본 필요 명중 수는 1이며, 한 명이라도 유효 피격되면 `COMBAT_RETREATING`이 완료되고 그룹이 이탈한다.

## 관찰 위치 조정

레벨의 `Gongsimdon_OBS_*` Actor를 실제 수풀, 적군, 봉돈 방향으로 이동한다. `RequiredViewTime`, `RequiredViewAngle`, `MaxDistance`, `bRequireLineOfSight`는 Actor별로 조정할 수 있다.
