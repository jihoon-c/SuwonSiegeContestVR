# 작업

신기전을 향해 돌진하는 40~50명 규모 적군을 Quest 3 VR 성능을 고려해 구현했다.

# 구현 내용

- `ASingijeonEnemyWaveActor` 추가
- 기본 45명, 3개 소대 자동 편성
- Spawn 위치와 화차 Target만 사용하는 단일 자동 Nav 경로
- NavMesh 경로 실패 시 직선 fallback
- 개별 AIController, Behavior Tree, Spline, Soldier Tick 제거
- 근거리 Shared Enemy Actor 최대 10명
- 원거리 HISM 대리체 35명
- 실체 병사는 매 프레임, HISM은 기본 15Hz 일괄 이동
- 화차 첫 장전 시 돌진 자동 시작
- 화차 일제 발사 완료 시 논리 병력 일괄 피격
- 전멸/목표 도달 상태와 Blueprint 이벤트 제공
- `LV_Singijeon`에 `Singijeon_EnemyWave` 배치

# 변경 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Enemy/SingijeonEnemyWaveActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonEnemyWaveTests.cpp
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/GF_Singijeon.Build.cs
Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap
Scripts/ConfigureSingijeonEnemyWave.py
Scripts/VerifySingijeonEnemyWave.py
Docs/Singijeon/specs/VR_INTERACTION.md
Docs/ARCHITECTURE.md
```

# 주요 결정 사항

- 공심돈의 6~8명 전용 Group은 변경하거나 참조하지 않았다.
- GF_Singijeon이 Shared Enemy/Combat을 사용하는 기존 의존 방향을 유지했다.
- 45개의 `ACharacter`를 만들지 않고 10 Actor + 35 HISM으로 분리했다.
- 대규모 단방향 연출이므로 Mass Crowd와 개별 Nav 이동을 도입하지 않았다.
- 역사 병사 VAT 자산이 없어 Proxy는 교체 가능한 임시 Static Mesh를 사용한다.

# 테스트 결과

- Win64 Development Editor 빌드: 성공
- Level 구성 검증: 전 항목 성공
- Enemy Wave 자동화: 성공
- GF_Singijeon 전체 자동화: 3/3 성공
- Map Check: 0 Error / 0 Warning

# 남은 문제

- 최종 역사 병사 Skeletal Mesh 및 VAT Proxy Mesh/Material 교체 필요
- Quest 3 실기기에서 GPU/프레임타임 프로파일링 필요
- 실제 지형 NavMesh와 적군 시작 위치는 아트 배치 완료 후 최종 튜닝 필요

