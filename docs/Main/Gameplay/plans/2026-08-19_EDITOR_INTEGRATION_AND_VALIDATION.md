# 목적

구현된 Shared Gameplay C++를 Unreal Editor Asset과 `GF_OngseongCrossbow` Level에 통합하고 Android 스탠드얼론 VR 성능을 검증한다.

# 현재 상태

Shared C++ 기반과 총통 Feature Runtime 모듈은 `bb37e29`에 구현됐다. 2026-08-20에 공용 Enemy Blueprint, 성문 목표, 적·투사체 Pool과 `AOngseongEnemyWaveManager`를 `LV_Ongseong`에 연결했다. UBT 빌드, 전체 Automation Test 10/10, MCP Simulate PIE와 Map Check 0/0을 통과했다. 남은 범위는 최종 시각/음향 자산, Muzzle/충돌 튜닝, 근거리 BT/StateTree, 체험 완료 조건과 Android 실기기 성능 검증이다.

# 구현 범위

- C++ 컴파일 완료
- 총통/포탄/적 Blueprint 생성 및 C++ 부모 클래스 연결
- 성문 목표 Actor, 총통, 목표점, Actor Pool을 `LV_Ongseong`에 배치하고 현재 비어 있는 참조를 연결
- Enemy 근거리 BT 또는 StateTree와 AI LOD 이벤트 연결
- Wave/Spawner Manager로 풀에서 적을 획득·반납
- Android 실기기 성능 측정 및 수치 조정

# 변경 예정 파일

- `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Gameplay/`
- `Content/Maps/LV_Ongseong.umap`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Data/`
- 필요 시 `GF_OngseongCrossbow` C++ 모듈

# 구현 단계

1. 총통/포탄/적 Blueprint를 만들고 총통 Muzzle, 포탄 시각·VFX를 설정한다.
2. 성문에 Health/Faction/Threat Component를 설정하고 Level 목표점과 Pool을 배치한다.
3. Wave/Spawner가 적의 목표·행동 상태를 설정하도록 연결한다.
4. Android 기기에서 CPU/GPU/메모리와 적·투사체 동시 수를 측정한다.

# 성능 기준

- Pool은 자동 확장을 끄고 시나리오의 동시 생성 상한만큼 사전 할당한다.
- 원거리 적은 메시와 BT/StateTree를 비활성화하고 단순 이동만 수행한다.
- 측정 없이 적 수, 풀 크기, LOD 거리, 발사 간격을 고정하지 않는다.

# 검증 방법

- UHT 및 Editor C++ 빌드 성공
- 총통 우선순위 3종을 Play In Editor에서 확인
- 공심돈 도주와 신기전 일방향 이동 확인
- Android 기기 성능 캡처 및 로그 확인
