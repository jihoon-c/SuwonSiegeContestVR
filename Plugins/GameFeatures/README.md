# Plugins/GameFeatures

이 디렉토리는 **Game Feature 계층**이다.

## 현재 상태 (2026-08-12)

`Status: Partial` — 구성 완료. 에셋 이름 1건 수정 필요.

| 항목 | 상태 |
|---|---|
| `.uplugin` 4개 | ✅ 생성 완료 |
| `.uproject`에 `GameFeatures` / `ModularGameplay` 활성화 | ✅ 완료 |
| 디렉토리 골격 (`Content/Gameplay`, `Phone`, `UI`, `Maps`, `Data`) | ✅ 완료 |
| `UGameFeatureData` 에셋 4개 | ⚠️ 생성됨. **`GF_Geojunggi` 1건 이름 불일치** (아래 참조) |
| **Asset Manager `PrimaryAssetTypesToScan` 등록** | ✅ `Config/DefaultGame.ini`에 추가 완료 |
| 실제 게임플레이 에셋 | ❌ 없음 |

### ⚠️ Asset Manager 등록은 필수다

`GameFeatureData`를 Primary Asset 타입으로 등록하지 않으면 시작 시 다음 경고가 나고
Game Feature가 동작하지 않는다.

```text
Asset Manager settings do not include an entry for assets of type
GameFeatureData, which is required for game feature plugins to function.
Add entry to PrimaryAssetTypesToScan?
```

`Config/DefaultGame.ini`의 `[/Script/Engine.AssetManagerSettings]` 항목이 이를 해결한다.
**이 항목을 지우면 4개 플러그인이 전부 무력화된다.**

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

## `UGameFeatureData` 에셋 — 이름이 플러그인과 정확히 같아야 한다

**각 플러그인에는 플러그인과 같은 이름의 `UGameFeatureData` 에셋이 Content 루트에 있어야 한다.**
이름이 한 글자라도 다르면 Game Features Subsystem이 해당 플러그인을 찾지 못한다.
**에러 없이 조용히 누락되므로 발견하기 어렵다.**

```text
Plugins/GameFeatures/GF_Geojunggi/Content/GF_Geojunggi.uasset
Plugins/GameFeatures/GF_OngseongCrossbow/Content/GF_OngseongCrossbow.uasset
Plugins/GameFeatures/GF_Gongsimdon/Content/GF_Gongsimdon.uasset
Plugins/GameFeatures/GF_Singijeon/Content/GF_Singijeon.uasset
```

### 현재 확인된 상태 (2026-08-12 파일 검증)

| 플러그인 | 에셋 내부 경로 | 판정 |
|---|---|---|
| `GF_Geojunggi` | `/GF_Geojunggi/GF_Geojung**gg**i` | ❌ **g가 하나 많음. 이름 불일치** |
| `GF_OngseongCrossbow` | `/GF_OngseongCrossbow/GF_OngseongCrossbow` | ✅ |
| `GF_Gongsimdon` | `/GF_Gongsimdon/GF_Gongsimdon` | ✅ |
| `GF_Singijeon` | `/GF_Singijeon/GF_Singijeon` | ✅ |

4개 모두 부모 클래스는 `/Script/GameFeatures.GameFeatureData`로 정상이다.

**수정 방법**: 에디터에서 `GF_Geojungggi` 에셋을 우클릭 → **Rename** → `GF_Geojunggi`.
참조가 없는 신규 에셋이므로 리다이렉터 정리는 불필요하다.

### 생성 절차 (신규 Feature 추가 시)

1. Content Browser 우측 상단 **Settings → Show Plugin Content** 활성화
2. 해당 플러그인 Content 루트로 이동
3. 우클릭 → **Miscellaneous → Data Asset** → 부모 클래스 **`GameFeatureData`** 선택
4. 플러그인과 **정확히 같은 이름**으로 저장
5. 에디터 재시작 후 로그 확인

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
