# 작업

신기전 적군 랜덤 편대와 GPU 스켈레탈 캐릭터 군중 적용

# 구현 내용

- `SM_MannequinTarget`과 원형 타깃 HISM 렌더링을 제거했다.
- 45명 중 근거리 피격용 3명은 실제 `AEnemySoldierActor`, 나머지 42명은 풀 Manny Skeletal Mesh GPU 인스턴스로 구성했다.
- 동일 달리기 애니메이션을 8개 시작 위상과 속도로 공유해 42명의 포즈 계산을 8개로 제한했다.
- GPU LOD, 5000~12000cm 컬링, 화면 크기 0.006 미만 애니메이션 정지, 무충돌·무그림자·무내비게이션을 적용했다.
- 편대의 좌우·전후·소대 중심·회전·크기·속도·애니메이션 위상을 Seed 기반으로 랜덤화했다.
- 간격의 42% 안에서만 흔들어 이웃 셀이 겹치지 않도록 제한했다.

# 변경 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Enemy/SingijeonEnemyWaveActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonEnemyWaveTests.cpp
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/Enemy/DA_SingijeonEnemyRun_GPU.uasset
Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap
Scripts/CreateSingijeonGpuEnemyAnimationProvider.py
Scripts/ConfigureSingijeonEnemyWave.py
Scripts/VerifySingijeonEnemyWave.py
docs/Singijeon/specs/VR_INTERACTION.md
docs/Singijeon/STATUS.md
```

# 주요 결정 사항

45개의 개별 Character/AI를 생성하지 않는다. 모든 병사는 캐릭터 외형을 유지하지만, 실제 충돌·체력 Actor는 가까운 3명만 사용해 Quest 3 CPU와 Draw Call 부하를 제한한다.

# 테스트 결과

- UE 5.8 `SuwonSiegeContestVREditor Win64 Development` 빌드 성공
- `SuwonSiegeContestVR.GF_Singijeon.EnemyWave.ScaleAndVolley` 성공
- `VerifySingijeonEnemyWave.py` 전체 항목 성공
- 레벨 설정에서 45명, 실제 Actor 3명, GPU 캐릭터 42명, 원형 Static Mesh 0개 확인

# 남은 문제

Quest 3 Android SDK가 현재 개발 PC에서 `INVALID r27c`로 감지되어 기기 패키징 성능 측정은 수행하지 못했다. VR Preview에서 외형을 확인한 뒤 Quest 3에서 `stat unit`, `stat gpu` 실측이 필요하다.
