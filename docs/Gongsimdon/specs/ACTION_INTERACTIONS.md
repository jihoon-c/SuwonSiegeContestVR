# 공심돈 나레이션·Action Interaction

`DA_Scenario_Gongsimdon` 한 곳에서 나레이션 27개와 동작 13개의 순서를 교차 정의한다. 나레이션 DT의 각 행은 `AdvanceMode=Stop`, `NextRow=None`이며, 다음 단계는 DT 체인이 아니라 Scenario의 `NextInteractionID`가 결정한다.

```text
NAR 01~04 → SCAN_PERIMETER
  → SOUND_ANIMAL → NAR 05 → CHECK_ANIMAL → NAR 06~08
  → SOUND_METAL → CHECK_METAL → REVEAL_ENEMY → NAR 09~10
  → IDENTIFY_ENEMY → NAR 11~13 → REPORT_ENEMY → NAR 14~16
  → ENABLE_BEACON → CHECK_BEACON → NAR 17~19
  → RETREAT_ENEMY → NAR 20~22 → SHOOT_RETREATING
  → NAR 23~24 → FINAL_SCAN → NAR 25~27
```

나레이션 데이터는 `/GF_Gongsimdon/Data/DT_Narration_Gongsimdon`, 음원은 `/GF_Gongsimdon/Asset/Narration`에 둔다. Level Scenario Manager에는 별도 DT를 중복 지정하지 않고 Experience를 통해 Scenario와 Scenario 소유 DT를 해석하게 한다.

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

기본 플레이에서는 배치된 `Gongsimdon_Report`의 원통형 보고 장치를 VR 트리거로 누르면 동쪽·6명 보고를 제출한다. 별도 보고 UI 또는 음성 인식 결과를 연결할 때는 다음 함수를 호출한다.

```text
SubmitReport(Direction, EnemyCount)
```

기본 정답은 `East`, 6~8명이다. 오답은 Scenario를 실패시키지 않고 현재 보고 단계를 유지한다.

## 사격 연결

배치된 `Gongsimdon_DefenseWeapon`을 손으로 조준하고 트리거를 누르면 50m Sweep과 조준 보조로 Health 대상을 찾고 표준 Damage를 전달한다. `Gongsimdon_EnemyGroup`이 Spawn한 병사 한 명이 유효 피격되면 `COMBAT_RETREATING`이 완료되고 그룹이 이탈한다.

## 관찰 위치 조정

레벨의 `Gongsimdon_OBS_*` Actor를 실제 수풀, 적군, 봉돈 방향으로 이동한다. `RequiredViewTime`, `RequiredViewAngle`, `MaxDistance`, `bRequireLineOfSight`는 Actor별로 조정할 수 있다.
