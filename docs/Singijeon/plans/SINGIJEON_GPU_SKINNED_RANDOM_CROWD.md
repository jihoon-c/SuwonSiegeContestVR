# 목적

**상태: 완료 (2026-08-21)**

신기전 적군의 정렬된 편대와 원형 타깃 프록시를 제거하고, Quest 3에서 유지 가능한 고품질 캐릭터 군중으로 교체한다.

# 현재 상태

- 적군 45명 중 3명만 `AEnemySoldierActor`이며 나머지는 `SM_MannequinTarget` HISM이다.
- 기존 편대 랜덤 오프셋은 좌우 30cm, 전후 20cm라 정배치처럼 보인다.
- 원형 형상은 `SM_MannequinTarget` 자체에 포함된 타깃 형상이다.

# 구현 범위

- 원형 Static Mesh 프록시 사용 중단
- 원거리 적군을 UE 5.8 `UInstancedSkinnedMeshComponent` 기반 풀 캐릭터로 렌더링
- 공유 GPU 애니메이션 위상, GPU LOD, 거리 컬링, 무충돌/무그림자 적용
- 편대 셀 안전 범위 안에서 위치·회전·크기·속도·애니메이션 위상 랜덤화
- 기존 피격/시나리오 판정은 근거리 실제 적군 액터와 논리 슬롯 구조로 유지

# 변경 예정 파일

- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Enemy/SingijeonEnemyWaveActor.*`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonEnemyWaveTests.cpp`
- `Scripts/ConfigureSingijeonEnemyWave.py`
- `Scripts/VerifySingijeonEnemyWave.py`
- `Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/Enemy/DA_SingijeonEnemyRun_GPU.uasset`

# 구현 단계

1. GPU 인스턴스 스켈레탈 컴포넌트와 캐릭터 메시를 연결한다.
2. 동일 달리기 애니메이션을 서로 다른 시작 위상으로 공유하는 Transform Provider를 생성한다.
3. 충돌 없는 seeded 랜덤 편대를 생성하고 인스턴스 이동을 10Hz로 갱신한다.
4. 레벨 액터를 새 시각 설정으로 저장하고 자동 검증한다.

# 다른 Feature에 미치는 영향

`GF_Singijeon` 내부 변경이며 Shared Enemy Soldier의 구현은 수정하지 않는다.

# 검증 방법

- C++ 빌드
- `GF_Singijeon.EnemyWave` 자동화 테스트
- 레벨 검증 스크립트로 45명/3 실제 액터/42 GPU 캐릭터, 원형 HISM 0개 확인
- 에디터 로드 로그에서 Blueprint/asset 오류 확인
