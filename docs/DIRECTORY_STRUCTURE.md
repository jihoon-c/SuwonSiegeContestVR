# DIRECTORY_STRUCTURE

**대상 프로젝트**: SuwonSiegeContestVR (Unreal Engine 5.8)
**조사 기준일**: 2026-08-11 (최초 조사) / **갱신일**: 2026-08-12 (계층 골격 구축)
**조사 기준 커밋**: `57dd875` (main / develop / feature/PawnSetting 모두 동일 커밋)
**타깃 기기**: **Android (스탠드얼론)** — 개발 중에는 PC에서 진행

> 이 문서는 **실제 조사 결과**와 **목표 구조**를 명확히 분리해서 기록한다.
> "현재 구조" 절에 적힌 것은 전부 파일 시스템에서 직접 확인한 내용이다.
> "목표 구조" 절은 아직 생성되지 않았다.

---

## 0. 요약

현재 리포지토리의 에셋은 **Unreal Engine 5.8 VR(XR) 템플릿의 기본 내용 그대로**이다.
2026-08-12 작업으로 **계층 골격 디렉토리가 생성**되었으나, 그 안에 실제 에셋은 아직 없다.
수원화성 VR 체험 관련 Level, Blueprint, Data Asset은 **하나도 존재하지 않는다.**

| 항목 | 현재 상태 |
|---|---|
| `Content/Core`, `Content/Gameplay`, `Content/Maps` | **골격 생성 완료** (빈 디렉토리 + `.gitkeep`) |
| `Plugins/GameFeatures/GF_*` | **골격 생성 완료.** 단 `.uplugin` 미생성 → 엔진이 플러그인으로 인식하지 않음 |
| Game Feature Plugin 실사용 여부 | **미결정** (`Plugins/GameFeatures/README.md` 참조) |
| `docs/` | 구축 완료 |
| Git LFS | **설정 완료** (신규 커밋부터 적용 — §4.2 참조) |
| C++ 게임플레이 코드 | **없음** (템플릿 스텁만 존재) |
| 프로젝트 전용 Level | **없음** — 사용자가 직접 생성 예정 |
| 실제 게임플레이 에셋 | **0개** |

---

## 1. 현재 루트 구조 (실제)

```text
D:\Github\SuwonSiegeContestVR\
│
├─ .git/
├─ .vs/                                  (VS 사용자 파일, .gitignore 대상)
├─ Config/
├─ Content/
├─ DerivedDataCache/                     (.gitignore 대상)
├─ Intermediate/                         (.gitignore 대상)
├─ Plugins/                              ★ 2026-08-12 생성 (골격만)
│  └─ GameFeatures/
├─ Saved/                                (.gitignore 대상)
├─ Source/
├─ docs/                                 ★ 프로젝트 문서
│
├─ .gitattributes                        (Git LFS 규칙 포함)
├─ .gitignore
├─ .vsconfig                             (untracked)
├─ CLAUDE.md                             (AI Agent 최상위 규칙)
├─ README.md                             (제목 한 줄만 존재)
├─ recommendation_project_directory.md   (untracked / 목표 구조 초안 — docs/로 대체됨)
├─ SuwonSiegeContestVR.uproject
├─ SuwonSiegeContestVR.sln               (.gitignore 대상)
├─ SuwonSiegeContestVR.slnx              (.gitignore 대상)
├─ Automation_SuwonSiegeContestVR.sln    (.gitignore 대상)
└─ Automation_SuwonSiegeContestVR.slnx   (.gitignore 대상)
```

### 루트 디렉토리 역할

| 디렉토리 | 역할 |
|---|---|
| `Config/` | 프로젝트 ini 설정. GameMode / 기본 맵 / 렌더링 / Enhanced Input 기본 IMC / Android(Quest) 패키징 설정 |
| `Content/` | 모든 Unreal Asset. 현재는 VR 템플릿 에셋만 |
| `Source/` | C++ 모듈. 현재 실질 구현 없음 |
| `DerivedDataCache/`, `Intermediate/`, `Saved/`, `.vs/` | 엔진/IDE 생성물. 버전 관리 대상 아님 |

---

## 2. 현재 `Content/` 구조 (실제)

```text
Content/
│
│  ===== 프로젝트 계층 골격 (2026-08-12 생성 · 전부 빈 디렉토리) =====
│
├─ Core/                         [Core 계층]
│  ├─ VR/            Pawn / Controller / Input / Interaction
│  ├─ PlayerPhone/   Actors / Components / UI / Interfaces
│  ├─ Experience/    Definitions / Progress
│  ├─ Quiz/          VoiceRecognition / InitialConsonant / Data / UI
│  └─ Interfaces/
│
├─ Gameplay/                     [Shared Gameplay 계층]
│  ├─ Characters/    Base / Enemy(Animation, Data) / Ally(Animation, Data)
│  ├─ Combat/        Health / Damage(DamageTypes, Interfaces) / Faction / Projectiles
│  ├─ AI/            Controllers / Behavior / Navigation / Spawning
│  ├─ UI/            Common / HUD / WorldSpace / Popup / Components
│  └─ Tags/
│
├─ Maps/
│  └─ Main/                      L_Main 배치 예정 (사용자가 직접 생성)
│
├─ Art/                          프로젝트 아트 에셋
├─ Audio/                        프로젝트 오디오 에셋
├─ Data/                         프로젝트 공통 Data Asset / Data Table
│
│  ===== 기존 (UE 템플릿 + 엔진 생성) =====
│
├─ Collections/                  (빈 디렉토리)
├─ Developers/
│  └─ Foryoucom/
│     └─ Collections/            (빈 디렉토리)
│
├─ LevelPrototyping/             [UE 템플릿] 프로토타이핑용 메시/머티리얼
│  ├─ Interactable/
│  │  ├─ Door/        BP_DoorFrame, SM_Door, SM_DoorFrame_*
│  │  ├─ JumpPad/     BP_JumpPad, NS_JumpPad, M_GradientGlow, ...
│  │  └─ Target/      BP_WobbleTarget, SM_TargetBaseMesh
│  ├─ Materials/      M_PrototypeGrid, MI_PrototypeGrid_*, MF_ProcGrid, ...
│  ├─ Meshes/         SM_Cube, SM_Cylinder, SM_Ramp, SM_ChamferCube, ...
│  └─ Textures/       T_GridChecker_A
│
├─ VRSpectator/                  [UE 템플릿] VR 관전자 카메라
│  ├─ VRSpectator.uasset, EVRSpectatorMode, RT_VRSpectator
│  └─ Input/
│     ├─ IMC_VRSpectator
│     └─ Actions/    IA_VRSpectator_Move / Look / FOV / Toggle / ...
│
├─ Weapons/                      [UE 템플릿] 총기 아트 에셋 (메시/머티리얼/텍스처만)
│  ├─ GrenadeLauncher/
│  ├─ Pistol/
│  └─ Rifle/
│
├─ XRFramework/                  [UE 템플릿] ★ 현재 프로젝트의 실질적 VR 프레임워크
│  ├─ Audio/          Fire01, Fire_Cue
│  ├─ Blueprints/
│  │  ├─ BP_XRPawn              VR Pawn (Camera / MotionController / Grab / Teleport)
│  │  ├─ BP_XRGameMode          DefaultPawnClass = BP_XRPawn
│  │  ├─ BP_GrabComponent       SceneComponent 파생, 잡기 컴포넌트
│  │  ├─ BP_Grabbable_SmallCube 잡을 수 있는 샘플 큐브
│  │  ├─ BP_Pistol              샘플 총기 (BP_Projectile 발사)
│  │  ├─ BP_Projectile          샘플 투사체 (Actor 파생, 데미지 시스템 없음)
│  │  ├─ BP_Menu / WBP_Menu     3D 메뉴 + 위젯
│  │  ├─ BP_TeleportVisualizer  텔레포트 목적지 표시
│  │  ├─ BP_Passthrough         패스스루 토글
│  │  ├─ BP_AssetGuideline      템플릿 에셋 가이드라인
│  │  ├─ BPI_PawnAnim           Pawn ↔ 손 애니메이션 인터페이스
│  │  └─ E_GrabType             Grab 방식 Enum
│  ├─ Haptics/        GrabHapticEffect, PistolFireHapticEffect
│  ├─ Input/
│  │  ├─ IMC_Default, IMC_Hands, IMC_Menu, IMC_Weapon_Left, IMC_Weapon_Right
│  │  ├─ BP_InputModifier_XAxisPositiveOnly
│  │  └─ Actions/     IA_Move, IA_Turn, IA_Grab_*, IA_Shoot_*, IA_Menu_*, Hands/IA_Hand_*
│  ├─ Levels/
│  │  ├─ L_XRTemplate.umap            ★ 현재 유일한 Level (기본 맵 / 에디터 시작 맵)
│  │  └─ L_XRTemplate_Lighting.umap   조명 서브레벨
│  ├─ Materials/      M_TeleportCylinder, M_VRCursor, MI_Grid_*, ...
│  ├─ Textures/       T_Grid
│  └─ VFX/            NS_TeleportTrace, NS_TeleportRing, NS_MenuLaser, NS_PlayAreaBounds
│
└─ XRMannequins/                 [UE 템플릿] VR 손/마네킹
   ├─ BP_MannequinsXR
   ├─ Animations/    A_MannequinsXR_Grasp/Idle/IndexCurl/Point/ThumbUp, MDT_MannequinsXR
   ├─ Materials/     M_Mannequin, MI_Manny_*, MI_Quinn_*
   ├─ Meshes/        ABP_MannequinsXR, SK_MannequinsXR, SKM_MannyXR_*, SKM_QuinnXR_*
   └─ Textures/      Manny/ Quinn/ Shared/
```

> **주의**: `Content/` 하위 전체(약 180개 에셋)에서 프로젝트 고유 에셋은 여전히 **0개**이다.
> 위 계층 골격은 디렉토리와 `.gitkeep`만 존재한다.
> `Content/Developers/Foryoucom/`은 존재하지만 에셋이 없다.

### Plugins 구조 (2026-08-12 생성)

```text
Plugins/
└─ GameFeatures/
   ├─ README.md                        ← 현재 상태 및 미결정 사항 기록
   ├─ GF_Geojunggi/Content/            Gameplay / Phone / UI / Maps / Data
   ├─ GF_OngseongCrossbow/Content/     Gameplay(Crossbow, SiegeRam, Waves, Experience) / Phone / UI / Maps / Data
   ├─ GF_Gongsimdon/Content/           Gameplay / Phone / UI / Maps / Data
   └─ GF_Singijeon/Content/            Gameplay / Phone / UI / Maps / Data
```

> **중요**: `.uplugin` 파일이 **없다.** 따라서 엔진은 이 디렉토리를 플러그인으로 인식하지 않으며,
> 여기에 에셋을 넣어도 마운트되지 않는다.
> Game Feature Plugin 실사용 여부가 확정될 때까지 **실제 에셋을 배치하지 않는다.**
> 자세한 내용은 `Plugins/GameFeatures/README.md` 참조.

---

## 3. 현재 `Source/` 구조 (실제)

```text
Source/
├─ SuwonSiegeContestVR.Target.cs
├─ SuwonSiegeContestVREditor.Target.cs
└─ SuwonSiegeContestVR/
   ├─ SuwonSiegeContestVR.Build.cs
   ├─ SuwonSiegeContestVR.h
   ├─ SuwonSiegeContestVR.cpp
   ├─ MyClass.h            ← 템플릿 스텁. UObject 아님, 어디서도 참조되지 않음
   └─ MyClass.cpp
```

`SuwonSiegeContestVR.Build.cs` 의존 모듈:

```text
PublicDependencyModuleNames  : Core, CoreUObject, Engine, InputCore
PrivateDependencyModuleNames : (비어 있음)
```

**상태**: C++ 모듈은 껍데기만 존재한다. `MyClass`는 `UCLASS`가 아닌 일반 C++ 클래스이며 프로젝트 어디에서도 사용되지 않는다.
`EnhancedInput`, `UMG`, `AIModule`, `GameplayTags`, `NavigationSystem` 등 향후 필요한 모듈은 아직 Build.cs에 추가되어 있지 않다.

> `SuwonSiegeContestVR.uproject`의 `"Modules"` 블록은 **현재 커밋되지 않은 로컬 수정 상태**이다 (`git diff` 확인됨).
> 즉 원격 `main`에는 아직 C++ 모듈 선언이 없다. 커밋 여부를 팀에서 결정해야 한다.

---

## 4. 현재 `Config/` 및 프로젝트 설정 (실제)

```text
Config/
├─ DefaultEditor.ini
├─ DefaultEngine.ini
├─ DefaultGame.ini
├─ DefaultInput.ini
├─ Android/AndroidEngine.ini
├─ Mac/MacEngine.ini
└─ VisionOS/VisionOSDeviceProfiles.ini, VisionOSEngine.ini
```

### 4.1 확인된 핵심 설정

| 설정 | 값 | 출처 |
|---|---|---|
| Engine Version | 5.8 | `.uproject` |
| GlobalDefaultGameMode | `/Game/XRFramework/Blueprints/BP_XRGameMode` | `DefaultEngine.ini` |
| GameDefaultMap / EditorStartupMap | `/Game/XRFramework/Levels/L_XRTemplate` | `DefaultEngine.ini` |
| bStartInVR | `True` | `DefaultGame.ini` |
| 활성 플러그인 | OpenXR, OpenXREyeTracker, OpenXRHandTracking, PICOController | `.uproject` |
| 렌더링 | Forward Shading, MobileMultiView, InstancedStereo, AA=3(TSR) | `DefaultEngine.ini` |
| Android 패키징 | `bPackageForMetaQuest=True`, quest2/questpro/quest3/quest3s, MinSDK 32 | `DefaultEngine.ini` |
| 기본 Input Mapping Context | IMC_Default, IMC_Hands, IMC_Weapon_Left, IMC_Weapon_Right, IMC_Menu | `DefaultInput.ini` |
| Enhanced Input | 활성 (`EnhancedPlayerInput` / `EnhancedInputComponent`) | `DefaultInput.ini` |
| GameplayTags 설정 | **없음** | 조사 결과 ini에 관련 섹션 없음 |
| Data Table / Data Asset | **없음** | 조사 결과 0개 |

### 4.2 확인된 버전 관리 설정

**Git LFS: 설정 완료 (2026-08-12)**

`.gitattributes`에 다음 계열을 LFS로 등록했다.

| 분류 | 확장자 |
|---|---|
| Unreal | `.uasset` `.umap` `.upk` `.udk` |
| 3D / DCC | `.fbx` `.abc` `.blend` `.ma` `.mb` `.max` `.3ds` |
| 텍스처 | `.png` `.jpg` `.tga` `.psd` `.exr` `.hdr` `.dds` `.tif` `.bmp` `.ico` |
| 오디오 | `.wav` `.mp3` `.ogg` `.flac` `.aif` `.aiff` |
| 비디오 | `.mp4` `.mov` `.webm` |
| 폰트 | `.ttf` `.otf` |
| 서드파티 바이너리 | `.so` `.a` `.aar` `.jar` `.dylib` `.framework` |

`git lfs install --local` 실행 완료 (post-checkout / post-commit / post-merge / pre-push hook 생성됨).

> ⚠️ **LFS의 적용 범위 — 반드시 이해할 것**
>
> LFS 규칙은 **설정 이후 add/commit 되는 파일에만** 적용된다.
> 이미 커밋된 기존 템플릿 에셋(약 180개)은 **일반 Git 오브젝트로 이력에 남아 있다.**
> 다만 앞으로 그 파일들을 **수정해서 커밋하면 그 시점부터 LFS로 전환**된다.
>
> 과거 이력까지 LFS로 옮기려면 `git lfs migrate import --everything`이 필요하며,
> 이는 **이력 재작성 + force push**를 수반해 다른 개발자의 clone을 무효화한다.
> 지금은 커밋이 2개뿐이라 마이그레이션 비용이 가장 낮은 시점이지만,
> **팀 합의 없이 실행하지 않는다.**

**`.gitignore` 갱신 사항 (2026-08-12)**

* `*.slnx` 추가 — UE 5.8이 생성하는 solution 파일이 untracked로 노출되던 문제 해결
* **ThirdParty 예외 규칙 추가** — 기존 `*.so` / `*.a` / `*.lib` / `*.dll` / `*.dylib` 무시 규칙이
  향후 임포트할 **서드파티 음성인식 SDK 바이너리(특히 Android `.so`)까지 차단**하는 문제가 있었다.
  `Source/**/ThirdParty/**`, `Plugins/**/Source/ThirdParty/**` 아래는 무시 대상에서 제외했다.
  → 서드파티 SDK는 반드시 **`ThirdParty/` 디렉토리 안에** 배치해야 커밋된다.

`.gitignore` 주요 항목: `.vs/`, `Binaries/*`, `Build/*`, `Saved/*`, `Intermediate/*`, `DerivedDataCache/*`, `*.sln`, `*.slnx`, `*_BuiltData.uasset`

`Plugins/**/Binaries/*`, `Plugins/**/Intermediate/*` 규칙은 이미 존재하므로 Game Feature Plugin 추가 시 별도 조치는 불필요하다.

---

## 5. 목표 구조 (Target)

> 아래는 `CLAUDE.md` 및 `recommendation_project_directory.md` 기준의 **목표 구조**이다.
> **2026-08-12 기준 디렉토리 골격은 전부 생성되었고, 내용물(에셋)이 비어 있다.**
> 신규 파일은 이 구조를 따라 배치한다.

```text
ProjectRoot/
├─ Config/
├─ Source/
│  └─ SuwonSiegeContestVR/        ← Core / Shared Gameplay C++
│
├─ Content/
│  ├─ Core/                       [골격 생성됨 / 내용 비어 있음]
│  │  ├─ VR/            Pawn / Controller / Input / Interaction
│  │  ├─ PlayerPhone/   Actors / Components / UI / Interfaces
│  │  ├─ Experience/    Definitions / Progress
│  │  ├─ Quiz/          VoiceRecognition / InitialConsonant / Data / UI
│  │  └─ Interfaces/
│  │
│  ├─ Gameplay/                   [골격 생성됨] (= Shared Gameplay 계층)
│  │  ├─ Characters/    Base(BP_CombatCharacter) / Enemy / Ally
│  │  ├─ Combat/        Health / Damage / Faction / Projectiles
│  │  ├─ AI/            Controllers / Behavior / Navigation / Spawning
│  │  ├─ UI/            Common / HUD / WorldSpace / Popup / Components
│  │  └─ Tags/
│  │
│  ├─ Maps/
│  │  └─ Main/L_Main            [Level은 사용자가 직접 생성 예정]
│  ├─ Art/ Audio/ Data/         [골격 생성됨]
│  │
│  └─ (XRFramework / XRMannequins / VRSpectator / Weapons / LevelPrototyping)
│      ← 기존 템플릿 에셋. 이동/삭제 전 반드시 참조 확인 필요
│
├─ Plugins/                       [골격 생성됨 — .uplugin 미생성]
│  └─ GameFeatures/
│     ├─ GF_Geojunggi/Content/         Gameplay / Phone / UI / Maps(L_Geojunggi) / Data
│     ├─ GF_OngseongCrossbow/Content/  Gameplay(Crossbow, SiegeRam, Waves, Experience) / Phone / UI / Maps(L_OngseongCrossbow) / Data
│     ├─ GF_Gongsimdon/Content/        Gameplay / Phone / UI / Maps(L_Gongsimdon) / Data
│     └─ GF_Singijeon/Content/         Gameplay / Phone / UI / Maps(L_Singijeon) / Data
│
└─ docs/                          ← 이번 작업으로 생성 완료
```

---

## 6. 현재 → 목표 차이 정리

| 기능 | 현재 위치 | 권장 위치 | 차이 | 향후 조치 |
|---|---|---|---|---|
| VR Pawn | `Content/XRFramework/Blueprints/BP_XRPawn` | `Content/Core/VR/Pawn/` | 템플릿 위치 그대로. 프로젝트 명명 규칙(`BP_VRPlayerPawn` 등) 미적용 | 프로젝트 전용 Pawn을 `Core/VR/Pawn`에 만들고 `BP_XRPawn`을 부모로 두거나 복제해 파생. **템플릿 원본 이동은 참조 파손 위험이 크므로 금지** |
| VR Input (IMC/IA) | `Content/XRFramework/Input/` | `Content/Core/VR/Input/` | 템플릿 위치. `DefaultInput.ini`가 이 경로를 직접 참조 | 이동 시 `DefaultInput.ini`의 `DefaultMappingContexts` 5줄을 반드시 함께 수정 |
| Grab / Interaction | `Content/XRFramework/Blueprints/BP_GrabComponent`, `E_GrabType` | `Content/Core/VR/Interaction/` | Grab만 존재. 공통 Interaction Interface 없음 | 공통 `BPI_Interactable` 정의 후 확장 |
| Teleport 이동 | `BP_XRPawn` 내부 + `BP_TeleportVisualizer` | `Content/Core/VR/Pawn/` | Pawn에 직접 구현되어 분리 안 됨 | 체험별 이동 방식이 갈리면 Component로 분리 검토 |
| GameMode | `Content/XRFramework/Blueprints/BP_XRGameMode` | `Content/Core/` | 템플릿 위치. `DefaultEngine.ini`가 직접 참조 | 프로젝트 전용 GameMode 신설 시 ini 동시 수정 필요 |
| Level | `Content/XRFramework/Levels/L_XRTemplate` | `Content/Maps/Main/L_Main` + 각 GF의 `Maps/` | 프로젝트 Level 전무 | `L_Main` 생성 후 `GameDefaultMap` 교체 |
| PlayerPhone | **없음** | `Content/Core/PlayerPhone/` | 미구현 | 신규 구현 |
| Experience / 진행도 | **없음** | `Content/Core/Experience/` + C++ Subsystem | 미구현 | 신규 구현 |
| Quiz / 음성 인식 | **없음** | `Content/Core/Quiz/` | 미구현 | 신규 구현. 음성 인식은 외부 플러그인/SDK 선정 필요 |
| Character / Health / Damage / Faction | **없음** | `Content/Gameplay/` + C++ Component | 미구현 | 신규 구현 |
| AI | **없음** | `Content/Gameplay/AI/` | 미구현. `AIModule` Build.cs 미포함 | 신규 구현 |
| Projectile | `BP_Projectile` (템플릿, 데미지 없음) | `Content/Gameplay/Combat/Projectiles/` | 데미지/Faction 연동 없는 샘플 | 공통 Projectile 신규 설계 |
| 공통 UI | `WBP_Menu` (템플릿 메뉴만) | `Content/Gameplay/UI/Common/` | 프로젝트 공통 위젯 없음 | 신규 구현 |
| Gameplay Tags | **없음** | `Content/Gameplay/Tags/` + ini | 미구현 | 태그 체계 사전 설계 권장 |
| Game Feature Plugin | **없음** (`Plugins/` 디렉토리 자체 없음) | `Plugins/GameFeatures/GF_*` | 4개 전부 미생성 | `GameFeatures` / `ModularGameplay` 플러그인 활성화 후 생성 |

---

## 7. 새 파일 생성 위치 규칙

작업 전 다음 순서로 판단한다.

```text
이 기능은 어느 계층인가?
   ├─ 프로젝트 전체 실행 기반인가?              → Core
   ├─ 둘 이상의 체험에서 재사용 가능한가?        → Shared Gameplay
   └─ 특정 체험 하나에서만 쓰이는가?            → 해당 Game Feature
```

### 7.1 계층별 배치 위치

| 계층 | Blueprint / Asset | C++ |
|---|---|---|
| Core | `Content/Core/<Subsystem>/` | `Source/SuwonSiegeContestVR/Core/` |
| Shared Gameplay | `Content/Gameplay/<Domain>/` | `Source/SuwonSiegeContestVR/Gameplay/` |
| Game Feature | `Plugins/GameFeatures/GF_<Name>/Content/<Domain>/` | 필요 시 해당 플러그인 모듈 |

### 7.2 자주 헷갈리는 케이스

| 만들려는 것 | 위치 |
|---|---|
| 적 병사, 아군 병사, 공통 전투 Character | `Content/Gameplay/Characters/` — **Game Feature 안에 넣지 않는다** |
| Health / Damage / Faction Component | `Content/Gameplay/Combat/` (+ C++ 권장) |
| 쇠뇌, 충차 | `Plugins/GameFeatures/GF_OngseongCrossbow/Content/Gameplay/` |
| 거중기 | `Plugins/GameFeatures/GF_Geojunggi/Content/Gameplay/` |
| 신기전 발사체 | 신기전 전용이면 `GF_Singijeon`, 공통 투사체 기반이면 기반은 `Content/Gameplay/Combat/Projectiles/` |
| PlayerPhone 본체 | `Content/Core/PlayerPhone/` |
| 체험별 Phone 기능 | `Plugins/GameFeatures/GF_*/Content/Phone/` (Component / Extension) |
| 여러 체험에서 쓰는 Widget | `Content/Gameplay/UI/Common/` |
| 특정 체험 전용 Widget | `Plugins/GameFeatures/GF_*/Content/UI/` |
| 체험 진행 Manager | 해당 Game Feature (`BP_*ExperienceManager`) |
| Level | Main → `Content/Maps/Main/`, 체험 → 해당 GF의 `Content/Maps/` |

### 7.3 금지 사항

* 템플릿 디렉토리(`XRFramework`, `XRMannequins`, `VRSpectator`, `Weapons`, `LevelPrototyping`)에 프로젝트 신규 에셋을 추가하지 않는다.
* `Content/Developers/` 아래 작업물을 공유 브랜치에 병합하지 않는다.
* `Content/` 루트에 파일을 직접 두지 않는다.
* Naming Prefix(`BP_`, `WBP_`, `BPC_`, `DA_`, `DT_`, `M_`, `MI_`, `SM_`, `SK_`, `ABP_`, `L_`, `SFX_`)는 `CLAUDE.md` 13절을 따른다.

---

## 8. 확인이 필요한 사항 (Needs Verification)

### 해결됨 (2026-08-12)

* ~~Git LFS 도입 여부~~ → **도입 완료.** §4.2 참조 (단, 과거 이력 마이그레이션은 미실행)
* ~~`*.slnx`가 ignore되지 않음~~ → **`.gitignore`에 추가 완료**
* ~~타깃 HMD 불명확~~ → **Android 스탠드얼론으로 확정.** 개발 중에는 PC에서 진행

### 미해결

* **`r.RayTracing=True`, `r.Substrate=True`** — 타깃이 Android로 확정된 만큼 이 설정은 **명확히 부적절하다.**
  Substrate는 모바일 지원이 제한적이고 RayTracing은 모바일에서 동작하지 않으면서 셰이더 컴파일 시간과 패키지 용량만 늘린다.
  `Config/DefaultEngine.ini`에서 정리해야 하나, 렌더링 결과와 빌드 파이프라인에 영향이 크므로 **별도 작업으로 분리**한다.
* `PICOController` 플러그인 활성 + Android 설정은 Quest 계열(quest2/questpro/quest3/quest3s) 명시 →
  실제 타깃 기기가 PICO인지 Quest인지, 혹은 양쪽 모두인지 확인 필요.
* `.uproject`의 `"Modules"` 로컬 수정을 커밋할지 — C++ 모듈 사용 여부 팀 결정 필요.
  단, 서드파티 음성인식 모듈 임포트가 예정되어 있으므로 **C++ 모듈은 결국 필요할 가능성이 높다.**
* `Content/Weapons/`(권총·소총·유탄발사기 아트)는 이 프로젝트 콘텐츠와 무관하다.
  Android 타깃에서는 패키지 용량이 곧 성능이므로 **제거를 권장**한다. 사용 계획 확인 필요.
* Game Feature Plugin 실사용 여부 — `Plugins/GameFeatures/README.md` 참조.
* LFS 과거 이력 마이그레이션(`git lfs migrate import`) 실행 여부 — 커밋이 적은 지금이 최적 시점.
