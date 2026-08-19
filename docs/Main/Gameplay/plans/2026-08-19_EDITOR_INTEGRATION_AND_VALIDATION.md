# 목적

구현된 Shared Gameplay C++를 Unreal Editor Asset과 `GF_OngseongCrossbow` Level에 통합하고 Android 스탠드얼론 VR 성능을 검증한다.

# 현재 상태

Shared C++ 기반과 총통 Feature Runtime 모듈은 `bb37e29`에 구현됐다. 최종 컴파일은 Editor Live Coding 때문에 아직 실행하지 못했다. 8000번 포트 MCP에는 에디터 Asset/Level/Blueprint 도구가 노출되지 않았다.

# 구현 범위

- Live Coding 종료 후 C++ 컴파일
- 총통/포탄/적 Blueprint 생성 및 C++ 부모 클래스 연결
- 성문 목표 Actor, 총통, 목표점, Actor Pool을 `LV_Ongseong`에 배치
- Enemy 근거리 BT 또는 StateTree와 AI LOD 이벤트 연결
- Wave/Spawner Manager로 풀에서 적을 획득·반납
- Android 실기기 성능 측정 및 수치 조정

# 변경 예정 파일

- `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Gameplay/`
- `Content/Maps/LV_Ongseong.umap`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Data/`
- 필요 시 `GF_OngseongCrossbow` C++ 모듈

# 구현 단계

1. Editor를 종료하거나 Live Coding을 종료한다.
2. `SuwonSiegeContestVREditor Win64 Development -NoHotReload`을 빌드한다.
3. 총통/포탄/적 Blueprint를 만들고 총통 Muzzle, 포탄 시각·VFX를 설정한다.
4. 성문에 Health/Faction/Threat Component를 설정하고 Level 목표점과 Pool을 배치한다.
5. Wave/Spawner가 적의 목표·행동 상태를 설정하도록 연결한다.
6. Android 기기에서 CPU/GPU/메모리와 적·투사체 동시 수를 측정한다.

# 성능 기준

- Pool은 자동 확장을 끄고 시나리오의 동시 생성 상한만큼 사전 할당한다.
- 원거리 적은 메시와 BT/StateTree를 비활성화하고 단순 이동만 수행한다.
- 측정 없이 적 수, 풀 크기, LOD 거리, 발사 간격을 고정하지 않는다.

# 검증 방법

- UHT 및 Editor C++ 빌드 성공
- 총통 우선순위 3종을 Play In Editor에서 확인
- 공심돈 도주와 신기전 일방향 이동 확인
- Android 기기 성능 캡처 및 로그 확인
