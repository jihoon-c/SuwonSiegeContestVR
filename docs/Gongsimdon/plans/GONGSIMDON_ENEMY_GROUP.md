# 목적

공심돈 야간 경계 Scenario에 실제 적군 6~8명을 배치하고 등장, 접근, 식별, 퇴각, 사격 판정을 연결한다.

**상태: 완료 (2026-08-19)**

# 현재 상태

- `CUE_REVEAL_ENEMY`, `OBS_ENEMY_GROUP`, `CUE_RETREAT_ENEMY`, `COMBAT_RETREATING` Interaction은 존재한다.
- 실제 적군 Character와 이동 Actor는 없다.
- Shared `EnemySoldier`, `HealthComponent`, `FactionComponent`는 문서상 Planned 상태다.

# 구현 범위

- Shared Health/Faction Component와 Enemy Soldier Actor 최소 구현
- 공심돈 전용 Enemy Group Actor와 접근/퇴각 Spline 구현
- 기본 7명, 3열 분산 Formation 구성
- 공심돈 Director Cue/Observe/Combat 자동 연결
- Level에 Enemy Group 배치 및 기존 임시 적 관찰/사격 Target 정리
- Manny 임시 Mesh와 보행 Anim BP 기본 적용

# 변경 예정 파일

```text
Source/SuwonSiegeContestVR/*/Shared/Combat/*
Source/SuwonSiegeContestVR/*/Shared/Characters/*
Plugins/GameFeatures/GF_Gongsimdon/Source/GF_Gongsimdon/*
Plugins/GameFeatures/GF_Gongsimdon/Content/Maps/LV_Gongsimdon.umap
Scripts/ConfigureGongsimdonEnemyGroup.py
Scripts/VerifyGongsimdonEnemyGroup.py
docs/ARCHITECTURE.md
docs/Gongsimdon/STATUS.md
```

# 구현 단계

1. Shared Health/Faction/Enemy Soldier 구현
2. Spline 기반 Enemy Group과 상태 머신 구현
3. 공심돈 Director 라우팅 연결
4. 자동화 테스트 및 Editor 빌드
5. Level 배치와 에셋 참조 검증
6. 문서 업데이트

# 다른 Feature에 미치는 영향

- Shared 적 병사는 향후 웅성/쇠뇌와 신기전에서도 재사용할 수 있다.
- Core Scenario는 변경하지 않는다.
- Manny는 임시 표시 자산이며 공심돈 Feature 콘텐츠를 Shared에 역참조하지 않는다.

# 검증 방법

- 기본 병사 수, Faction, Health 자동화 테스트
- Reveal/Approach/Retreat/Combat 상태 전환 테스트
- Win64 Development Editor 빌드
- Level Enemy Group 재조회
- 전체 프로젝트 자동화 테스트

# 구현 결과

- Shared `AEnemySoldierActor`, `UHealthComponent`, `UFactionComponent` 구현
- 기본 7명, 3열 대형의 공심돈 Enemy Group 구현 및 Level 배치
- Reveal/Retreat Cue, Observe, Combat 자동 연결
- 기존 정적 적 관찰/사격 자리표시자 제거
- Win64 Development Editor 빌드 성공
- 전체 프로젝트 자동화 9/9 성공
