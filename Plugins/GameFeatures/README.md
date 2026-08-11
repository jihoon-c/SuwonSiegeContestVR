# Plugins/GameFeatures

이 디렉토리는 **Game Feature 계층의 골격**이다.

## 현재 상태 (2026-08-11 기준)

`Status: Skeleton Only` — **디렉토리만 존재하며 `.uplugin` 파일은 생성되지 않았다.**

즉 이 시점에는 Unreal Engine이 이 디렉토리를 **플러그인으로 인식하지 않는다.**
`.uplugin`이 없으면 엔진이 무시하므로 프로젝트 실행에는 아무 영향이 없다.

## 미결정 사항

Game Feature Plugin(Modular Gameplay)을 실제로 사용할지, 아니면 단순 Content 폴더 분리로 갈지 **아직 확정되지 않았다.**

* GFP를 사용하려면 `.uproject`에 `GameFeatures`, `ModularGameplay` 플러그인을 활성화해야 한다.
* 추가로 `UGameFeatureData` 기반 Data Asset과 로딩/활성화 액션 설계가 필요하다.
* 현재 `.uproject`의 활성 플러그인은 OpenXR, OpenXREyeTracker, OpenXRHandTracking, PICOController 4개뿐이다.

결정 전까지는 각 `GF_*/Content/` 아래에 에셋을 배치해도 **일반 Content 폴더처럼 동작하지 않는다**(엔진이 마운트하지 않음).
따라서 **결정이 날 때까지 이 디렉토리에 실제 에셋을 넣지 않는다.**

## 결정 후 절차

1. `.uproject`에 `GameFeatures`, `ModularGameplay` 활성화
2. 각 `GF_*`에 `.uplugin` 생성 (`"Type": "Runtime"`, `"ExplicitlyLoaded": true`)
3. 각 플러그인에 `UGameFeatureData` Data Asset 생성
4. `docs/DIRECTORY_STRUCTURE.md`, `docs/ARCHITECTURE.md`, 해당 `docs/<Feature>/STATUS.md` 갱신

## 의존성 규칙

```text
Game Feature  →  Shared Gameplay (Content/Gameplay)  →  Core (Content/Core)
```

* Game Feature 간 직접 참조 금지 (`GF_A` → `GF_B`)
* Core / Shared Gameplay가 Game Feature를 참조하는 것 금지

상세 규칙은 `docs/COLLABORATION.md` §8 참조.

## 계층 배치 주의

적 병사, Health / Damage / Faction, AI, 공통 Projectile은 **여러 체험이 공유하므로 Shared Gameplay(`Content/Gameplay/`)에 둔다.**
이 디렉토리에는 **해당 체험에서만 쓰이는 것**(거중기, 쇠뇌, 충차, 신기전 발사대, 공심돈 탐지 로직)만 넣는다.
