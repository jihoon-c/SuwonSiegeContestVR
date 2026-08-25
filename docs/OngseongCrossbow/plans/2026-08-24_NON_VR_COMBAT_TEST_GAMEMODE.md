# 비VR 전투 테스트 GameMode

**작성일**: 2026-08-24 · **대상 Feature**: `GF_OngseongCrossbow` (+ Core 신규 디버그 클래스)
**상태**: `Implemented (PIE 육안 확인 대기)` — C++·에셋·테스트 레벨 완료. 하단 "진행 상태" 참조.

---

# 목적

VR HMD와 VR Player Pawn 없이 PC 에디터에서 다음을 관찰·검증할 수 있는 별도 GameMode와
테스트 레벨을 만든다.

* 아군 총통(`BP_AllyChongtong`)과 적 궁병의 원거리 교전
* 충차(`AOngseongRamActor`)의 성문 접근·돌진·충돌 피해 반복
* 적 스폰/리스폰 루프와 Pool 동작 (`2026-08-24_SUSTAINED_ENEMY_POPULATION` 계획과 함께 사용)

플레이어는 어떤 게임플레이 Actor에도 빙의하지 않고, 관찰용 자유 카메라만 조작한다.

---

# 현재 상태

| 항목 | 확인 내용 |
|---|---|
| 전역 GameMode | `Config/DefaultEngine.ini`의 `GlobalDefaultGameMode=/Game/XRFramework/Blueprints/BP_XRGameMode`. `DefaultPawnClass = BP_VRPlayerPawn` |
| `LV_Ongseong` | World Settings에 GameMode Override 없음 → PIE 시 VR Pawn 생성, HMD 연결 시 스테레오 자동 활성 |
| 플레이어 총통·쇠뇌 | `UChongtongAimGripComponent` / `UOngseongCrossbowGripComponent`가 양손 그립 + 양손 트리거를 요구 → VR 입력 없이는 조작 불가 |
| `AOngseongDefenseScenarioManager` | `bAutoStart=true`. `VRHUD`는 첫 PlayerController의 Pawn에서 찾고 없으면 `nullptr`이며 모든 사용처가 null 가드를 가진다(`OngseongDefenseScenarioManager.cpp:32`, `118`, `152`, `240`) → VR Pawn이 없어도 크래시 없음 |
| `AOngseongEnemyWaveManager` | `bAutoStart=true`. `ArcherPrimaryTarget`이 비어 있으면 `Ongseong.ArcherTarget` 태그 → `AChongtongCannonActor` 순으로 자동 탐색 → 아군 총통만 배치되어 있으면 VR Pawn 없이 교전 성립 |
| 성공 처리 | `bReturnToMainOnSuccess=true`면 `UExperienceSubsystem::CompleteCurrentExperience`로 Main 복귀를 요청 → 테스트 레벨에서는 꺼야 한다 |
| 디버그 전용 GameMode/Pawn | 프로젝트에 없음 |

즉, **전투 로직 자체는 VR Pawn에 의존하지 않는다.** 막고 있는 것은 GameMode가 항상 VR Pawn을
생성한다는 점과, 테스트용 자유 카메라가 없다는 점뿐이다.

---

# 구현 범위

## Core (재사용 가능한 디버그 도구)

* `ADebugFreeCameraPawn : ADefaultPawn`
  * 충돌 비활성, 중력 없음, `UFloatingPawnMovement` 기반 자유 비행
  * 기본 속도 1200 cm/s, 가속 키(Shift) ×4, 감속 키(Ctrl) ×0.25
  * 마우스 룩 + WASD + E/Q 상하
* `ADebugFreeCameraGameMode : AGameModeBase`
  * `DefaultPawnClass = ADebugFreeCameraPawn`, `HUDClass = nullptr`
  * `bDisableHMDOnBeginPlay`(기본 true): `UHeadMountedDisplayFunctionLibrary::EnableHMD(false)`
  * `PlayerStart`가 없으면 `SpawnLocationOverride` 위치에 카메라 Pawn을 생성

Core에 두는 이유: 신기전·공심돈도 같은 방식의 비VR 전투 확인이 필요하며, 이 클래스는 어떤
Game Feature도 참조하지 않는다. 의존 방향은 `Feature → Core`만 생긴다.

## Feature (`GF_OngseongCrossbow`)

* `BP_OngseongCombatTestGameMode` — Core GameMode의 자식. 옹성 테스트 전용 디버그 키 바인딩
  (예: `R` 시나리오 재시작, `1` 적 스폰 일시정지/재개)만 Blueprint로 추가한다.
* `LV_Ongseong_CombatTest` — `LV_Ongseong` 복제본. World Settings에서 위 GameMode를 Override 한다.
  * 배치 인스턴스 조정: `bReturnToMainOnSuccess=false`, `DefenseDuration`을 테스트용으로 조정 가능
  * 플레이어 총통(`BP_PlayableChongtong`)은 남겨두되 조작하지 않는다. 아군 총통·궁병·충차·성문·Pool은 그대로 유지한다.

## 범위 밖

* VR 전용 상호작용(플레이어 총통 장전, 쇠뇌 조준)의 마우스/키보드 대체 조작. 필요하면 별도 계획으로 다룬다.
* `LV_Ongseong` 본편 레벨 수정. 본편은 이번 작업에서 건드리지 않는다.
* `Config/DefaultEngine.ini`의 `GlobalDefaultGameMode` 변경. ini가 경로를 문자열로 참조하므로
  (`docs/COLLABORATION.md` 참조) 전역 설정 대신 레벨 Override만 사용한다.

---

# 변경 예정 파일

| 파일 | 종류 |
|---|---|
| `Source/SuwonSiegeContestVR/Public/Core/Debug/DebugFreeCameraPawn.h` | 신규 |
| `Source/SuwonSiegeContestVR/Private/Core/Debug/DebugFreeCameraPawn.cpp` | 신규 |
| `Source/SuwonSiegeContestVR/Public/Core/Debug/DebugFreeCameraGameMode.h` | 신규 |
| `Source/SuwonSiegeContestVR/Private/Core/Debug/DebugFreeCameraGameMode.cpp` | 신규 |
| `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Blueprints/Debug/BP_OngseongCombatTestGameMode.uasset` | 신규 |
| `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Maps/LV_Ongseong_CombatTest.umap` | 신규 |
| `docs/OngseongCrossbow/ARCHITECTURE.md`, `STATUS.md` | 갱신 |

---

# 구현 단계

1. Core에 `ADebugFreeCameraPawn`과 `ADebugFreeCameraGameMode`를 추가하고 Editor 타깃을 빌드한다.
2. 에디터에서 `BP_OngseongCombatTestGameMode`를 만들고 부모를 `DebugFreeCameraGameMode`로 지정한다.
3. `LV_Ongseong`을 `LV_Ongseong_CombatTest`로 복제하고 World Settings에서 GameMode를 Override 한다.
4. 테스트 레벨의 `BP_OngseongDefenseScenarioManager` 인스턴스에서 `bReturnToMainOnSuccess=false`로 설정한다.
5. PIE(VR Preview 아님)로 실행해 VR Pawn이 생성되지 않고 자유 카메라가 조종되는지 확인한다.
6. 궁병 → 총통 사격, 총통 → 적 사격, 충차 → 성문 피해가 모두 발생하는지 관찰한다.
7. 문서를 갱신한다.

---

# 다른 Feature에 미치는 영향

* Core에 **신규 클래스만 추가**하며 기존 Core 클래스(`AVRPlayerPawn`, `BP_XRGameMode`)는 수정하지 않는다.
* Core → Feature 역의존은 발생하지 않는다. 테스트 GameMode는 옹성 클래스를 참조하지 않으며,
  옹성 전용 설정은 Feature의 Blueprint 자식과 테스트 레벨에만 존재한다.
* 다른 Feature도 같은 Core GameMode를 상속해 자체 비VR 테스트 레벨을 만들 수 있다.

---

# 검증 방법

* `SuwonSiegeContestVREditor Win64 Development` 빌드
* PIE에서 다음 확인
  * `BP_VRPlayerPawn` 인스턴스 0개
  * WASD/마우스로 카메라 이동, 어떤 Actor에도 빙의하지 않음
  * 궁병 화살과 총통 포탄이 발사되고 Pool에 반환됨
  * 충차가 성문 Health를 감소시킴
* 기존 `SuwonSiegeContestVR.Ongseong` Automation 4종이 그대로 통과하는지 확인
* `LV_Ongseong` 본편의 GameMode 설정이 변경되지 않았는지 확인

---

# 위험 요소

* `ADefaultPawn`은 엔진 기본 축 바인딩(`MoveForward`, `Turn` 등)을 사용한다. 프로젝트가
  Enhanced Input 전용 설정으로 이 바인딩을 무시하면 카메라가 움직이지 않는다.
  → 이 경우 Core에 `IMC_DebugFreeCamera` + `IA_` 에셋을 추가해 Enhanced Input으로 대체한다.
  구현 1단계에서 먼저 확인한다.
* 테스트 레벨은 본편 레벨의 복제본이므로, 본편이 크게 바뀌면 수동 동기화가 필요하다.
  테스트 레벨은 "전투 검증용"으로만 유지하고 최종 콘텐츠 기준으로 삼지 않는다.

---

# 진행 상태 (2026-08-24)

## 구현 완료 (C++)

* `Source/SuwonSiegeContestVR/Public|Private/Core/Debug/DebugFreeCameraPawn.h/.cpp`
  * `ADefaultPawn` 상속, 충돌 비활성, 자유 비행
  * 입력은 축 매핑이나 IMC 없이 `APlayerController::IsInputKeyDown` / `GetInputMouseDelta` 폴링으로 처리한다.
    엔진 기본 축 바인딩(`bAddDefaultMovementBindings`)은 끄고 `SetupPlayerInputComponent`에서 Super를 호출하지 않는다.
    → 계획서 "위험 요소"에 적었던 Enhanced Input 의존 문제를 설계로 제거했다.
  * 조작: WASD/방향키 이동, E·Space 상승, Q 하강, 마우스 시점, Shift 가속(×4), Ctrl 감속(×0.25)
* `Source/SuwonSiegeContestVR/Public|Private/Core/Debug/DebugFreeCameraGameMode.h/.cpp`
  * `DefaultPawnClass = ADebugFreeCameraPawn`, `HUDClass = nullptr`
  * `bDisableHMDOnBeginPlay`: `GEngine->StereoRenderingDevice->EnableStereo(false)` 사용.
    `HeadMountedDisplayFunctionLibrary.h`는 XRBase **플러그인** 모듈에 있어 Core가 플러그인에 의존하게 되므로 사용하지 않았다.
  * `bCaptureMouseOnBeginPlay`: 커서 숨김 + `FInputModeGameOnly`

## 남은 작업 (에디터 필요)

`Scripts/CreateOngseongCombatTestMode.py`를 에디터에서 실행하면 아래가 한 번에 처리된다.

1. `BP_OngseongCombatTestGameMode` 생성 (부모 `DebugFreeCameraGameMode`)
2. `LV_Ongseong` → `LV_Ongseong_CombatTest` 복제
3. 테스트 레벨 World Settings의 GameMode Override 지정
4. 시나리오 매니저 인스턴스 `bReturnToMainOnSuccess=false`
5. Pool 크기 재산정과 `Ram_ActorPool` 배치/연결

실행 전 **에디터 타깃 빌드가 필요하다.** 현재 에디터가 Live Coding 상태여서 에디터 타깃 링크가 차단되어 있다.

## 검증 상태

* `SuwonSiegeContestVR Win64 Development` (Game 타깃): **성공** — 컴파일·링크·UHT 통과
* `SuwonSiegeContestVREditor Win64 Development`: **보류** — "Unable to build while Live Coding is active"
* PIE 확인(VR Pawn 미생성, 자유 카메라 조작): **미실행**

---

# 진행 상태 갱신 (2026-08-24, 2차)

## 에디터 에셋 작업 완료

`Scripts/CreateOngseongCombatTestMode.py`를 헤드리스로 실행해 아래를 생성·저장했다.

* `/GF_OngseongCrossbow/Blueprints/Debug/BP_OngseongCombatTestGameMode` (부모 `DebugFreeCameraGameMode`)
* `/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest` (`LV_Ongseong` 복제본)
* 테스트 레벨 World Settings의 GameMode Override
* `Ongseong_DefenseScenario` 인스턴스의 `bReturnToMainOnSuccess=false`, `RamPool` 연결
* `Ram_ActorPool`(크기 2) 배치, `Enemy_ActorPool` 9 / `Archer_ActorPool` 8 / `Ongseong_RangedProjectilePool` 24로 조정
* `DebugCamera_PlayerStart` — 관전 카메라가 원점이 아니라 아군 총통 옆(성문 방향)에서 시작하도록 배치

## 구동 검증 (헤드리스 `-game`)

```text
LogLoad: Game class is 'BP_OngseongCombatTestGameMode_C'
LogOngseong: Defense started. Ram=BP_OngseongRam_C_2, enemy slots=15, time limit=off
LogOngseong: Ongseong population filled: 15 living enemies.
LogActorPool 고갈 경고 0건
```

VR Pawn은 생성되지 않았고 레벨은 정상 로드된다.

## 남은 작업

* 사람이 PIE로 열어 자유 카메라 조작감(WASD/마우스/속도)과 시야를 확인
* 필요 시 `DebugCamera_PlayerStart` 위치·각도 조정
