# Plugins/GameFeatures

이 디렉토리는 **Game Feature 계층**이다.

## 현재 상태 (2026-08-12)

`Status: Partial` — `.uplugin` 생성 완료. **`UGameFeatureData` 에셋 미생성.**

| 항목 | 상태 |
|---|---|
| `.uplugin` 4개 | ✅ 생성 완료 |
| `.uproject`에 `GameFeatures` / `ModularGameplay` 활성화 | ✅ 완료 |
| 디렉토리 골격 (`Content/Gameplay`, `Phone`, `UI`, `Maps`, `Data`) | ✅ 완료 |
| **`UGameFeatureData` 에셋 4개** | ❌ **미생성 — 에디터에서 직접 만들어야 한다** |
| 실제 게임플레이 에셋 | ❌ 없음 |

### 생성된 플러그인

```text
GF_Geojunggi           거중기 체험
GF_OngseongCrossbow    웅성·쇠뇌 체험
GF_Gongsimdon          공심돈 체험
GF_Singijeon           신기전 체험
```

공통 `.uplugin` 설정:

```json
"CanContainContent": true,
"ExplicitlyLoaded": true,
"BuiltInInitialFeatureState": "Registered"
```

`ExplicitlyLoaded: true`는 Game Feature Plugin의 필수 설정이다.
엔진이 시작 시 콘텐츠를 자동 마운트하지 않고, Game Features Subsystem이 상태를 제어하게 한다.

---

## ⚠️ 남은 필수 작업 — `UGameFeatureData` 에셋 생성

**각 플러그인에는 플러그인과 같은 이름의 `UGameFeatureData` 에셋이 Content 루트에 있어야 한다.**

```text
Plugins/GameFeatures/GF_Geojunggi/Content/GF_Geojunggi.uasset
Plugins/GameFeatures/GF_OngseongCrossbow/Content/GF_OngseongCrossbow.uasset
Plugins/GameFeatures/GF_Gongsimdon/Content/GF_Gongsimdon.uasset
Plugins/GameFeatures/GF_Singijeon/Content/GF_Singijeon.uasset
```

이 에셋은 **바이너리 `.uasset`이라 에디터에서만 생성할 수 있다.**
없으면 Game Features Subsystem이 시작 시 해당 플러그인을 건너뛰며 로그에 에러를 남긴다.

### 생성 절차

1. 에디터를 켜고 Content Browser 우측 상단 **Settings → Show Plugin Content** 활성화
2. 각 플러그인 Content 루트로 이동
3. 우클릭 → **Miscellaneous → Data Asset** → 부모 클래스 **`GameFeatureData`** 선택
4. 플러그인과 **정확히 같은 이름**으로 저장 (예: `GF_Geojunggi`)
5. 4개 플러그인 모두 반복
6. 에디터 재시작 후 로그에 Game Feature 관련 에러가 없는지 확인

> 대안: UE 에디터의 **New Plugin → Game Feature (Content Only)** 마법사를 쓰면
> `.uplugin`과 `GameFeatureData`가 함께 생성된다. 다만 이미 `.uplugin`이 있으므로
> 이름 충돌을 피하려면 위 수동 절차를 권장한다.

---

## 검증 필요 (Needs Verification)

`.uplugin`을 손으로 작성했으므로 **에디터에서 다음을 실제로 확인해야 한다.**

* 4개 플러그인이 **Plugins 창에 나타나는가**
* 시작 로그에 Game Feature 관련 **에러/경고가 없는가**
* Content Browser에서 각 플러그인 Content 폴더가 **마운트되는가**
  (`ExplicitlyLoaded: true`이므로 Feature가 Active 상태가 되기 전에는 마운트되지 않는 것이 정상이다)
* `BuiltInInitialFeatureState`를 `Registered`로 두었다. 체험 진입 시 `Active`로 올리는 흐름은
  `ExperienceSubsystem` 설계와 함께 결정한다.

문제가 있으면 이 README와 `docs/ARCHITECTURE.md`를 갱신한다.

---

## 에셋 배치 규칙

**`UGameFeatureData` 에셋이 생성되기 전까지는 이 디렉토리에 실제 에셋을 넣지 않는다.**
Feature가 Active가 되지 않으면 콘텐츠가 마운트되지 않아 참조가 깨질 수 있다.

### 의존성 규칙

```text
Game Feature  →  Shared Gameplay (Content/Gameplay)  →  Core (Content/Core)
```

* Game Feature 간 직접 참조 금지 (`GF_A` → `GF_B`)
* Core / Shared Gameplay가 Game Feature를 참조하는 것 금지

상세 규칙은 `docs/COLLABORATION.md` §8 참조.

### 계층 배치 주의

적 병사, Health / Damage / Faction, AI, 공통 Projectile은 **여러 체험이 공유하므로 Shared Gameplay(`Content/Gameplay/`)에 둔다.**
이 디렉토리에는 **해당 체험에서만 쓰이는 것**(거중기, 쇠뇌, 충차, 신기전 발사대, 공심돈 탐지 로직)만 넣는다.
