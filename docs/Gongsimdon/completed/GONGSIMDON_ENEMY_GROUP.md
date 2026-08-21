# 작업

공심돈 야간 경계에 실제 적군 7명의 등장, 접근, 관찰, 퇴각, 사격 판정을 구현했다.

# 구현 내용

- Shared Enemy Soldier, Health, Faction 기반 추가
- 공심돈 전용 7명/3열 Spline Enemy Group 구현
- `CUE_REVEAL_ENEMY`에서 접근 시작
- 이동 그룹 중심으로 `OBS_ENEMY_GROUP` 관찰
- `CUE_RETREAT_ENEMY`에서 퇴각 시작
- 개별 병사 표준 Damage를 그룹 Combat 완료로 연결
- 기존 정적 적 관찰/사격 Placeholder 제거
- 공심돈 Level에 Enemy Group 배치

# 변경 파일

```text
Source/SuwonSiegeContestVR/Public/Shared/Combat/*
Source/SuwonSiegeContestVR/Private/Shared/Combat/*
Source/SuwonSiegeContestVR/Public/Shared/Characters/EnemySoldierActor.h
Source/SuwonSiegeContestVR/Private/Shared/Characters/EnemySoldierActor.cpp
Plugins/GameFeatures/GF_Gongsimdon/Source/GF_Gongsimdon/*/Enemy/GongsimdonEnemyGroupActor.*
Plugins/GameFeatures/GF_Gongsimdon/Content/Maps/LV_Gongsimdon.umap
Scripts/ConfigureGongsimdonEnemyGroup.py
Scripts/VerifyGongsimdonEnemyGroup.py
```

# 주요 결정 사항

- 병사는 Shared, 공심돈 대형과 연출은 Feature에 배치했다.
- Quest 3에서 예측 가능하고 가벼운 Spline 이동을 사용했다.
- 적 하나 이상 명중하면 사격 Interaction을 완료한다.
- 역사 병사 자산이 없어 Manny Mesh를 임시 사용한다.
- 문제를 일으키는 Niagara 예제 Anim Blueprint는 기본 참조하지 않는다.

# 테스트 결과

- Win64 Development Editor 빌드: 성공
- Enemy Group Level 구성 검증: 전 항목 성공
- 공심돈 Action Interaction 재검증: 성공
- Spawn 7명, Enemy Faction, Health, 접근, 퇴각, 명중 완료 테스트: 성공
- 전체 `SuwonSiegeContestVR` 자동화: 9/9 성공

# 남은 문제

- 실제 조선시대 적군 Mesh와 Animation Blueprint 필요
- Level 지형에 맞춘 Approach/Retreat Spline 조정 필요
- 실제 무기/투사체의 Hit에서 Soldier `ApplyDamage` 연결 필요
- Quest 3에서 Skeletal Mesh 7명 성능과 관찰 거리 튜닝 필요
