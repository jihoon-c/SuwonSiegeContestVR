# 목적

Quest 3 VR에서 신기전을 향해 돌진하는 적군 40~50명을 개별 Spline, AIController,
Behavior Tree 없이 운용한다.

# 상태

완료

# 현재 상태

- Shared Gameplay에는 `AEnemySoldierActor`, Health, Faction이 있다.
- 공심돈에는 6~8명 전용 `AGongsimdonEnemyGroupActor`가 있으나 GF_Gongsimdon 전용이다.
- 신기전에는 대규모 적군 Wave와 자동 경로가 없다.
- 역사 병사 전용 최종 Mesh/Animation 자산은 아직 없다.

# 구현 범위

- `GF_Singijeon` 전용 Enemy Wave Actor
- Spawn 영역과 Target만으로 Nav 경로 1회 자동 생성
- 기본 45명, 3개 소대 자동 편성
- 매니저 하나만 Tick하는 이동
- 최대 10명의 Shared Enemy Soldier와 35명의 HISM 시각 대리체
- 화차 일제 발사 이벤트에 대한 논리 피격 판정
- Blueprint 조정 변수와 상태/완료 이벤트
- `LV_Singijeon` 자동 배치 및 검증 스크립트

# 변경 예정 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Enemy/SingijeonEnemyWaveActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/GF_Singijeon.Build.cs
Scripts/ConfigureSingijeonEnemyWave.py
Scripts/VerifySingijeonEnemyWave.py
Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap
Docs/Singijeon/*
```

# 구현 단계

1. 논리 병력 Slot과 자동 경로 API를 구현한다.
2. 근거리 실체 병사와 원거리 HISM 대리체를 분리한다.
3. 단일 Wave Tick에서 실체 병사는 매 프레임, HISM은 제한 주기로 이동한다.
4. 화차 Volley와 도달/전멸 이벤트를 연결한다.
5. Level 배치, 빌드, 자동화 테스트, 구성 검증을 수행한다.

# 다른 Feature에 미치는 영향

- 공심돈 Enemy Group은 변경하지 않는다.
- GF_Singijeon은 Shared Enemy/Combat만 참조한다.
- Core 또는 Shared가 GF_Singijeon을 참조하지 않는다.

# 검증 방법

- 기본 병력 45명, 실체 10명, 대리체 35명 확인
- 경로 실패 시 직선 경로 fallback 확인
- Start/Stop/도달/Volley 피격 상태 자동화 테스트
- Editor 빌드와 `LV_Singijeon` 배치 검증
