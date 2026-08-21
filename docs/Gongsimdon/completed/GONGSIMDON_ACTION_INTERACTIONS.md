# 작업

공심돈 야간 경계 스토리에서 나레이션을 제외한 Action Interaction과 판정 클래스를 구현했다.

# 구현 내용

- Content-only였던 `GF_Gongsimdon`에 Runtime 모듈 추가
- 정적 관찰 5개, 이동 Enemy Group 관찰 1개, Sequence/Spawn 5개, 보고 1개, 사격 1개로 구성된 13단계 Scenario 작성
- Scenario 요청을 Level Actor에 자동 연결하는 Director 구현
- HMD 응시 기반 Observation Target 구현
- 동쪽/6~8명 보고 판정 구현
- Enemy Soldier 표준 Damage 기반 사격 판정 구현
- Level에 필요한 판정 Actor 기초 배치

# 변경 파일

```text
Plugins/GameFeatures/GF_Gongsimdon/GF_Gongsimdon.uplugin
Plugins/GameFeatures/GF_Gongsimdon/Source/GF_Gongsimdon/*
Plugins/GameFeatures/GF_Gongsimdon/Content/Data/DA_Scenario_Gongsimdon.uasset
Plugins/GameFeatures/GF_Gongsimdon/Content/Maps/LV_Gongsimdon.umap
Scripts/ConfigureGongsimdonActionInteractions.py
Scripts/VerifyGongsimdonActionInteractions.py
docs/Gongsimdon/specs/ACTION_INTERACTIONS.md
```

# 주요 결정 사항

- 대사와 자막은 이번 범위에서 제외했다.
- 공심돈 전용 동작은 Feature 모듈에 두고 Core Scenario에는 역의존성을 추가하지 않았다.
- 오답 보고는 Scenario 실패로 처리하지 않고 현재 Interaction을 유지한다.
- 연출 위치를 알 수 없는 상태이므로 Observation Target은 PlayerStart 기준 기초 위치로 배치했다.

# 테스트 결과

- Win64 Development Editor 빌드: 성공
- Action Interaction 에셋/Level 검증: 전 항목 성공
- 보고 오답 유지, 정답 진행, 사격 필요 명중 수 테스트: 성공
- 전체 `SuwonSiegeContestVR` 자동화: 8/8 성공

# 남은 문제

- 실제 레벨 메시 기준으로 관찰 타깃과 Enemy Group Spline을 조정해야 한다.
- 음향, 적군, 봉돈 시각 연출을 Director Cue 이벤트에 연결해야 한다.
- 보고 UI/음성 인식과 실제 사격 무기를 Shared Soldier Damage에 연결해야 한다.
- Quest 3에서 HMD 관찰 각도와 거리 튜닝이 필요하다.
