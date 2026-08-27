# 작업

옹성 전투의 적 초기 스폰, 병사 리스폰, 충차 스폰 위치를 Blueprint Actor로 저작할 수 있게 하고 총통 발사·포탄 폭발 피드백을 Blueprint에서 교체 가능하게 했다.

# 구현 내용

- `AOngseongSpawnPointActor`를 추가했다.
  - `EnemyInitial`, `SoldierRespawn`, `Ram` 역할을 인스턴스별로 선택한다.
  - 에디터에서 역할별 텍스트와 색상 화살표를 표시하고 게임에서는 숨긴다.
- `/GF_OngseongCrossbow/Gameplay/Waves/BP_OngseongSpawnPoint`를 생성했다.
- `AOngseongEnemyWaveManager`가 최초 병력 배치에는 `EnemyInitial`, 사망 후 보충에는 `SoldierRespawn`을 사용한다.
- `AOngseongDefenseScenarioManager`가 명시적 `RamSpawnPoint`가 없을 때 `Ram` 역할 포인트를 자동 탐색한다.
- 역할 포인트가 배치되지 않은 기존 레벨은 종전처럼 매니저 Transform을 사용하므로 기존 배치와 호환된다.
- 총통의 `MuzzleEffect`, `FireSound`와 포탄의 `ExplosionEffect`, `ExplosionSound`를 `BlueprintReadWrite`로 노출했다.

# 변경 파일

- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/OngseongSpawnPointActor.h`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Ongseong/OngseongSpawnPointActor.cpp`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/OngseongEnemyWaveManager.h`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Ongseong/OngseongEnemyWaveManager.cpp`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Private/Ongseong/OngseongDefenseScenarioManager.cpp`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/ChongtongCannonActor.h`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/Public/Ongseong/ChongtongProjectileActor.h`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Gameplay/Waves/BP_OngseongSpawnPoint.uasset`
- `Scripts/CreateOngseongSpawnPointBlueprint.py`

# 주요 결정 사항

- 세 지점을 하나의 복합 액터에 고정하지 않고 동일 Blueprint의 역할별 인스턴스로 만들었다. 레벨별로 각 지점을 독립 이동·회전할 수 있고 필요한 역할만 명시적으로 참조할 수 있다.
- 기존 레벨을 자동 수정하거나 기존 Actor Transform을 바꾸지 않았다. 새 포인트가 배치될 때만 자동 탐색 경로가 활성화된다.
- 발사 피드백은 `BP_AllyChongtong`과 `BP_PlayableChongtong`, 폭발 피드백은 두 총통이 공통으로 사용하는 `BP_ChongtongProjectile`에서 관리한다.

# 사용 방법

1. `BP_OngseongSpawnPoint`를 레벨에 세 번 배치한다.
2. 각 인스턴스의 `Ongseong > Spawning > Spawn Point Role`을 `EnemyInitial`, `SoldierRespawn`, `Ram`으로 각각 지정한다.
3. 필요하면 웨이브 매니저의 `Initial Spawn Point`, `Soldier Respawn Point`와 시나리오 매니저의 `Ram Spawn Point`에 직접 연결한다. 비워 두면 역할로 자동 탐색한다.
4. 포구 화염·포격음은 `BP_AllyChongtong` 또는 `BP_PlayableChongtong`의 Class Defaults > `Ongseong|Chongtong|Feedback`에서 바꾼다.
5. 폭발 이펙트·폭발음은 `BP_ChongtongProjectile`의 같은 Feedback 카테고리에서 바꾼다.

# 테스트 결과

- UE 5.8 UHT 및 C++ 컴파일 성공. 실행 중 에디터의 기본 DLL 잠금을 피하기 위해 suffix 모듈로 최종 링크했고 새 클래스를 로드했다.
- `BP_OngseongSpawnPoint`, `BP_AllyChongtong`, `BP_PlayableChongtong`, `BP_ChongtongProjectile`를 warnings-as-errors로 컴파일: 성공.
- `BP_OngseongSpawnPoint` 부모: `/Script/GF_OngseongCrossbow.OngseongSpawnPointActor` 확인.
- `SuwonSiegeContestVR.Ongseong` 자동화 5종: 전부 성공, 종료 코드 0.
- 기존 임시 `Fire_Cue`의 one-shot 경고와 의도적으로 비어 있는 Pool 설정 경고는 기존 테스트의 알려진 경고이며 실패는 아니다.

# 남은 문제

- 실제 좌표는 레벨 디자인 결정이므로 기존 레벨 Actor를 임의 이동하지 않았다. 레벨에서 역할별 인스턴스 3개를 배치해야 새 위치가 적용된다.
- 최종 Niagara/SoundCue 에셋 선정과 HMD 육안·청각 검증은 콘텐츠 작업으로 남는다.
