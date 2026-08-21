# Gongsimdon (공심돈) — 현재 상태

**조사 기준일**: 2026-08-19

**계층**: Game Feature — `GF_Gongsimdon` (+ Core Scenario/Experience 의존)

**전체 상태**: `Status: Partial` — 레벨 진입, 야간 경계 Action, 7명 적군 접근·퇴각, Main 자동 복귀 구현. 역사 병사 자산·음향·보고 UI·무기는 연결 대기

## 현재 구현

| 요소 | 상태 | 실제 경로/내용 |
|---|---|---|
| Game Feature | `Implemented` | `Plugins/GameFeatures/GF_Gongsimdon` |
| Level | `Partial` | `/GF_Gongsimdon/Maps/LV_Gongsimdon`, PlayerStart와 Scenario Manager 설정 |
| Scenario | `Implemented` | `/GF_Gongsimdon/Data/DA_Scenario_Gongsimdon`, 나레이션 없는 야간 경계 Action 13단계 |
| Experience | `Implemented` | `/Game/Core/Experience/Definitions/DA_Experience_Gongsimdon`, 완료 시 `L_Main` 복귀 |
| Main 연동 | `Implemented` | `MAIN_TRAVEL_GONGSIMDON`에서 진입, `MAIN_AFTER_GONGSIMDON`부터 복원 |
| 탐지/관측 게임플레이 | `Implemented` | 정적 관찰 타깃 5개 + 이동 Enemy Group 관찰과 자동 Scenario 라우팅 |
| 적군 그룹 | `Implemented` | Shared Enemy Soldier 7명, 3열 대형, 접근/퇴각 Spline, 1회 명중 완료 |
| 적 보고 판정 | `Implemented` | 동쪽, 6~8명 정답 판정. UI/음성 호출 연결 대기 |
| 퇴각 적 사격 판정 | `Implemented` | 개별 Soldier에 표준 `ApplyDamage` 시 그룹 Combat 완료. 무기 입력 연결 대기 |

## 현재 흐름

```text
경계 구역 관찰
  → 동물 소리 방향 확인
  → 쇳소리 방향 확인
  → 적 등장/식별
  → 위치·규모 보고
  → 봉돈 신호 확인
  → 퇴각 적 사격
  → 최종 경계 관찰
  → L_Main / MAIN_AFTER_GONGSIMDON
```

진행 기록은 `UExperienceSubsystem`의 세션 체크포인트에 저장된다. 앱을 종료하면 사라지며 SaveGame 영속화는 아직 없다.

## 후속 작업

- 공심돈 구조물과 실제 연출 위치에 Observation Target 배치 조정
- 동물/쇳소리와 봉돈 신호를 Director의 Cue 이벤트에 연결
- Manny 임시 Mesh를 조선시대 적군 Skeletal Mesh/Animation Blueprint로 교체
- 보고 UI 또는 음성 인식에서 `SubmitReport` 호출
- 실제 무기/투사체에서 개별 Enemy Soldier에 `ApplyDamage` 전달
- 전용 나레이션 테이블과 자막 연결
- Quest 3 PIE/실기에서 공심돈 완료 → Main 복귀 → 신기전 진입 전체 동선 검증

## 의존성 원칙

- 공심돈 고유 탐지 로직은 `GF_Gongsimdon`에 둔다.
- 재사용 적, Faction, AI는 Shared Gameplay를 사용한다.
- Core는 공심돈 클래스를 직접 참조하지 않고 Scenario/Experience Data Asset으로 연결한다.
