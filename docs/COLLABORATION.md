# COLLABORATION

**대상 프로젝트**: SuwonSiegeContestVR (Unreal Engine 5.8)
**조사 기준일**: 2026-08-11
**조사 기준 커밋**: `57dd875`

이 문서는 여러 개발자가 동시에 작업할 때의 협업 규칙을 정의한다.
`CLAUDE.md` 14절(협업 규칙)의 구체적 실행 지침이며, 충돌 시 `CLAUDE.md`가 우선한다.

---

## 1. 현재 확인된 리포지토리 상태

> 아래는 실제 조사 결과이며, 추측한 내용은 포함하지 않는다.

| 항목 | 확인된 값 |
|---|---|
| Remote | `origin` → `https://github.com/jihoon-c/SuwonSiegeContestVR.git` |
| 기본 브랜치 | `main` (`origin/HEAD` → `origin/main`) |
| 원격 브랜치 | `main`, `develop`, `feature/PawnSetting` |
| 브랜치 상태 | **세 브랜치 모두 동일 커밋 `57dd875`를 가리킨다.** 분기된 작업 없음 |
| 커밋 이력 | `0585be5 Initial commit` → `57dd875 프로젝트 업로드` (총 2개) |
| Git LFS | **설정 완료 (2026-08-12).** §6.5 참조 |

### 관측된 브랜치 네이밍

`develop`, `feature/PawnSetting` 브랜치가 존재한다는 사실로부터
**`main` / `develop` / `feature/*` 형태의 Git Flow 계열 네이밍을 사용하려는 의도**가 확인된다.

> **주의**: 실제 병합 이력이나 브랜치 정책 문서는 존재하지 않는다.
> 아래 §2는 관측된 네이밍에 근거한 **제안**이며, 팀이 확정하기 전까지는 확정 규칙이 아니다.
> 확정되면 이 문서를 갱신한다.

---

## 2. Git Workflow

### 확정된 규칙 (2026-08-12)

* **통합 브랜치는 `develop`이다.** 작업 결과는 `develop`에 커밋·푸시한다.
* **`main`에 직접 커밋하지 않는다.** `main`은 안정 상태 유지용이다.

### 제안 (팀 확정 필요)

아래 세부 사항은 아직 확정되지 않았다.

```text
main            배포/검증 가능한 안정 상태
  └─ develop    통합 브랜치. 모든 feature가 여기로 병합
       ├─ feature/<Feature명>-<작업>     예: feature/OngseongCrossbow-EnemyWave
       ├─ feature/Core-<작업>            예: feature/Core-PlayerPhone
       └─ feature/Shared-<작업>          예: feature/Shared-HealthComponent
```

### 제안 규칙

* 브랜치명에 담당 계층/Feature를 넣어 충돌 범위를 한눈에 파악할 수 있게 한다.
* 작업 브랜치는 오래 유지하지 않는다. Unreal 바이너리 에셋은 자동 병합이 불가능하므로 **장기 브랜치일수록 충돌 해결 비용이 기하급수적으로 커진다.**
* 병합 전 `develop`을 먼저 가져와 로컬에서 충돌을 해소한다.

### 확정 필요 사항 (TODO)

* PR(Pull Request) 필수 여부 및 리뷰어 수
* `develop` → `main` 승격 기준
* 커밋 메시지 규칙 (현재 이력 2개로는 규칙을 판단할 수 없다)

---

## 3. Feature별 작업 분리 기준

작업을 시작하기 전에 **자신이 건드리는 계층**을 먼저 확정한다.

```text
이 작업은 ...
├─ 특정 체험 하나에서만 쓰이는가?          → 해당 Game Feature. 단독 작업 가능
├─ 둘 이상의 체험에서 쓰일 수 있는가?      → Shared Gameplay. 사전 공유 필요
└─ 프로젝트 전체 실행 기반인가?            → Core. 사전 합의 필수
```

### 담당 범위 원칙

| 계층 | 작업 권한 | 사전 협의 |
|---|---|---|
| Game Feature (`GF_*`) | 담당자가 자유롭게 수정 | 불필요 |
| Shared Gameplay | 담당자 지정 권장 | 인터페이스 변경 시 필수 |
| Core | 담당자 지정 필수 | **모든 변경에 대해 필수** |

### 현재 상태에서의 주의점

* 아직 계층 디렉토리(`Content/Core`, `Content/Gameplay`)와 Game Feature Plugin이 **하나도 없다.**
* 따라서 **각 개발자가 서로 다른 위치에 파일을 만들면 즉시 구조가 어긋난다.**
* 첫 구현 착수 전에 `docs/DIRECTORY_STRUCTURE.md` §5의 골격 디렉토리를 만들고 한 번에 커밋해 기준을 고정하는 것을 강력히 권장한다.

---

## 4. 공통 시스템 수정 규칙

`Content/Core/`, `Content/Gameplay/`, `Source/` 아래를 수정할 때 적용한다.

### 수정 전 확인

1. `docs/ARCHITECTURE.md`에서 해당 시스템의 상태와 설계 의도를 확인한다.
2. `docs/<Feature>/plans/`, `docs/<Feature>/completed/`에서 진행 중이거나 완료된 관련 작업을 확인한다.
3. Unreal Editor의 **Reference Viewer**로 해당 에셋을 참조하는 모든 곳을 확인한다.
4. C++이라면 헤더를 include하는 모든 파일을 확인한다.

### 변경 시 금지

* 공통 Component / Interface의 **함수 시그니처를 사전 공유 없이 변경**하지 않는다.
* 공통 Blueprint의 변수·핀을 삭제하거나 이름을 바꾸지 않는다 (참조하는 Blueprint가 조용히 깨진다).
* 공통 Enum의 항목 **순서를 바꾸거나 중간에 삽입**하지 않는다 (직렬화된 값이 어긋난다). 추가는 항상 끝에 한다.
* Core 또는 Shared Gameplay에서 특정 Game Feature를 직접 참조하지 않는다. 필요하면 Interface / Event Dispatcher / Gameplay Tag / Data Asset으로 역전한다.

### 변경 후 필수 확인

```text
1. Compile Error 확인 (C++ / Blueprint)
2. Blueprint Compile 경고 확인
3. Reference Viewer로 참조 파손 확인
4. 영향받는 모든 Feature의 Level을 한 번씩 열어 에러 로그 확인
5. docs/ARCHITECTURE.md 갱신 필요 여부 판단
```

---

## 5. Blueprint 충돌 방지

Unreal의 `.uasset`은 **바이너리이며 Git이 자동 병합할 수 없다.** 충돌하면 한쪽을 버려야 한다.

### 필수 규칙

* **같은 Blueprint를 두 사람이 동시에 수정하지 않는다.** 시작 전에 반드시 팀에 알린다.
* 한 Blueprint에 기능을 몰아넣지 말고 **Actor Component 단위로 쪼갠다.** 서로 다른 Component를 편집하면 충돌하지 않는다.
* 큰 작업 전에 `git pull`, 작업 후 **가능한 한 빨리 push**한다. 로컬에 오래 들고 있을수록 위험하다.
* 공용 Blueprint를 **내용 변경 없이 재저장하지 않는다.** 엔진은 열고 닫기만 해도 파일을 변경할 수 있으므로, 의도치 않은 diff는 커밋에서 제외한다.
* 충돌이 발생하면 임의로 `--ours` / `--theirs`를 고르지 말고 **양쪽 작업자가 함께 결정**한다.

### 권장 사항

* 데이터성 설정은 Blueprint 변수 기본값 대신 **Data Asset / Data Table**로 분리한다. Data Asset도 바이너리지만 편집 주체가 명확히 나뉜다.
* 로직이 자주 바뀌지 않는 공통 기반(Health, Faction, Damage Interface, Experience Subsystem)은 **C++로 작성**한다. 텍스트라 병합이 가능하다. (`CLAUDE.md` 12절)

---

## 6. Level 수정 규칙

`.umap`도 바이너리이며 충돌 시 병합이 불가능하다. **Level은 가장 충돌이 잦은 자산이다.**

### 필수 규칙

* **한 Level은 한 시점에 한 사람만 편집한다.** 편집 시작·종료를 팀에 알린다.
* Level Blueprint에 게임플레이 로직을 넣지 않는다. `BP_*ExperienceManager`에 구현한다 (`CLAUDE.md` 7절).
  → 로직이 Manager Blueprint에 있으면 Level을 열지 않고도 수정할 수 있어 충돌이 줄어든다.
* 다른 사람 담당 Level의 배치 액터를 임의로 옮기거나 삭제하지 않는다.
* `L_XRTemplate`(템플릿 맵)은 참고용으로만 두고, 프로젝트 작업은 프로젝트 Level에서 한다.

### 권장 사항

* 여러 명이 한 Level을 동시에 다뤄야 하면 **Level Streaming(서브레벨)** 또는 **World Partition + Data Layer**로 담당 구역을 분리한다.
* `*_BuiltData.uasset`은 `.gitignore` 대상이므로 라이팅 빌드 결과는 공유되지 않는다. 각자 로컬에서 빌드한다.

### `_BuiltData` 경고는 정상이다 (무시해도 됨)

클론 직후 에디터를 열면 다음과 같은 경고가 나온다. **버그가 아니라 의도된 결과다.**

```text
While trying to load package /Game/XRFramework/Levels/L_XRTemplate, a dependent
package /Game/XRFramework/Levels/L_XRTemplate_BuiltData was not available.
... Perhaps it has been deleted or was not synced?
```

원인: `.gitignore`의 `*_BuiltData.uasset` 규칙 때문에 라이팅 빌드 데이터가 리포지토리에 없다.
`_BuiltData`는 용량이 크고 라이팅을 빌드할 때마다 통째로 바뀌는 바이너리라, 커밋하면
리포지토리가 급격히 커지고 충돌도 잦아진다. 그래서 의도적으로 제외한다.

* 조명이 어색해 보이면 **Build → Build Lighting Only**를 로컬에서 한 번 실행한다.
* 프로토타이핑 단계에서는 그냥 무시해도 무방하다.
* 이 경고를 없애려고 `.gitignore`에서 `*_BuiltData.uasset`을 빼지 않는다.

---

## 6.5 Git LFS 운영 규칙

**설정 완료일**: 2026-08-12 · 상세 규칙은 `.gitattributes`, `docs/DIRECTORY_STRUCTURE.md` §4.2 참조.

### 개발자 최초 1회 설정 (필수)

리포지토리를 clone하거나 pull하기 전에 각자 로컬에서 실행한다.

```bash
git lfs install
```

이 명령을 실행하지 않으면 `.uasset` / `.umap` 파일이 **실제 에셋이 아니라 몇 줄짜리 텍스트 포인터 파일로 받아진다.**
그 상태로 프로젝트를 열면 에셋이 전부 깨져 보인다. 증상이 나타나면:

```bash
git lfs install
git lfs pull
```

### 규칙

* LFS 대상 확장자를 임의로 추가·삭제하지 않는다. 변경이 필요하면 팀에 공유한 뒤 `.gitattributes`를 수정한다.
* **서드파티 SDK 바이너리는 반드시 `ThirdParty/` 디렉토리 안에 배치한다.**
  `.gitignore`의 `*.so` / `*.a` / `*.lib` / `*.dll` / `*.dylib` 규칙이 그 밖의 위치에서는 파일을 차단한다.
* 커밋 전 `git lfs status`로 LFS 대상이 제대로 잡혔는지 확인하는 습관을 들인다.

### ⚠️ 이력 마이그레이션 완료 (2026-08-12) — 전원 재클론 필요

`git lfs migrate import --everything --include="*.uasset,*.umap"`을 실행해
**과거 이력 전체를 LFS로 전환**했다. 커밋 187개 에셋 전부가 LFS 포인터로 바뀌었다.

**이력이 재작성되어 모든 커밋 SHA가 변경되었다.** 원격에는 force push로 반영된다.

| 브랜치 | 이전 SHA | 이후 SHA |
|---|---|---|
| `main` | `57dd875` | `0d63b55` |
| `develop` | `520b44f` | `a3cd291` |
| `feature/PawnSetting` | `57dd875` | `0d63b55` |

반영 명령 (마이그레이션 수행자가 1회 실행):

```bash
git push --force-with-lease origin develop main feature/PawnSetting
```

#### 다른 개발자가 해야 할 일 (필수)

기존 로컬 클론은 **더 이상 원격과 호환되지 않는다.** `git pull`은 실패하거나 이력을 뒤섞는다.

**권장: 재클론**

```bash
git lfs install
git clone https://github.com/jihoon-c/SuwonSiegeContestVR.git
```

**로컬에 커밋하지 않은 작업이 있다면**, 먼저 그 변경분을 별도 위치에 백업(파일 복사 또는 `git diff > patch`)한 뒤
재클론하고 수동으로 다시 적용한다. **기존 클론에서 `git pull`이나 `git merge`를 시도하지 않는다.**

#### 되돌리기

마이그레이션 직전 전체 상태는 `pre-lfs-migration.bundle`로 백업되어 있다(세션 스크래치패드).
복구가 필요하면 이 번들에서 원래 이력을 복원할 수 있다. 필요 시 담당자에게 요청한다.

#### 참고

* GitHub 원격의 **옛 오브젝트는 즉시 사라지지 않는다.** GitHub 측 GC 이후에 정리되므로
  원격 리포지토리 용량 감소는 지연되어 반영된다.
* 앞으로는 LFS 대상 파일이 자동으로 LFS에 저장되므로 추가 조치가 필요 없다.

---

## 7. Asset 이동 및 Rename 규칙

Unreal에서 에셋을 옮기거나 이름을 바꾸면 **Redirector**가 생기고, 다른 사람이 그 사이 작업 중이면 참조가 깨진다.

### 규칙

* 불필요한 이동·Rename을 하지 않는다.
* 반드시 이동해야 하면:
  1. 팀에 사전 공지하고, 가능하면 **다른 작업이 없는 시점**에 한다.
  2. Editor의 **Move / Rename 기능**으로만 옮긴다. **탐색기에서 파일을 직접 옮기지 않는다.**
  3. `Fix Up Redirectors in Folder`를 실행해 Redirector를 정리한다.
  4. 이동과 Redirector 정리를 **하나의 커밋으로** 함께 올린다.
* **템플릿 디렉토리(`XRFramework`, `XRMannequins`, `VRSpectator`, `Weapons`, `LevelPrototyping`)의 에셋은 이동하지 않는다.**
  `DefaultEngine.ini`(GameMode, 기본 맵)와 `DefaultInput.ini`(IMC 5개)가 이 경로를 **문자열로 직접 참조**한다.
  이동 시 ini를 함께 수정하지 않으면 프로젝트가 실행되지 않는다.
* `Content/Developers/` 아래 개인 작업물은 공유 브랜치에 병합하지 않는다. 공유가 필요하면 정식 위치로 옮긴 뒤 커밋한다.

---

## 8. Game Feature Dependency 규칙

허용되는 의존 방향은 하나뿐이다.

```text
Game Feature  →  Shared Gameplay  →  Core
```

| 허용 | 금지 |
|---|---|
| `GF_Singijeon` → `EnemySoldier` | `Core` → `GF_OngseongCrossbow` |
| `GF_OngseongCrossbow` → Damage System | `EnemySoldier` → `GF_Singijeon` |
| `GF_Gongsimdon` → 공통 UI | 공통 Widget → `GF_Geojunggi` |
| — | `GF_A` → `GF_B` (Feature 간 직접 참조) |

### 역의존이 필요해 보일 때

Core/Shared에서 특정 Feature를 참조해야 할 것 같으면, 그건 대개 **설계가 잘못된 신호**다.
다음 순서로 검토한다.

1. **Interface** — Core가 Interface를 정의하고 Feature가 구현
2. **Event Dispatcher** — Core가 이벤트를 쏘고 Feature가 바인딩
3. **Gameplay Tag** — 구체 클래스 대신 태그로 분기
4. **Component** — Feature가 Core 액터에 Component를 주입 (PlayerPhone 확장 방식)
5. **Data Asset** — Feature가 데이터를 등록, Core는 데이터만 소비

그래도 해결되지 않으면 **혼자 결정하지 말고 팀에 공유**한다.

---

## 9. 공통 클래스 변경 시 확인 사항 (체크리스트)

공통 Blueprint / C++ 클래스를 수정할 때 아래를 순서대로 확인한다.

```text
[ ] 이 클래스를 참조하는 모든 에셋을 Reference Viewer로 확인했는가
[ ] 영향받는 Feature를 전부 나열했는가
[ ] 함수 시그니처 / 변수 / Enum 항목을 삭제·변경하지 않았는가 (추가만 했는가)
[ ] 삭제·변경이 불가피하다면 해당 Feature 담당자에게 알렸는가
[ ] C++이라면 Build.cs 의존 모듈 추가가 필요한가
[ ] Blueprint Compile 에러 / 경고가 없는가
[ ] 영향받는 Level을 열어 로그 에러를 확인했는가
[ ] Core → Feature 역의존을 만들지 않았는가
[ ] docs/ARCHITECTURE.md 갱신이 필요한가
[ ] docs/DIRECTORY_STRUCTURE.md 갱신이 필요한가
```

---

## 10. 문서 업데이트 규칙

코드와 문서를 서로 다른 상태로 방치하지 않는다 (`CLAUDE.md` 16절).

### 반드시 문서를 함께 수정해야 하는 변경

| 변경 내용 | 갱신 대상 |
|---|---|
| 디렉토리 구조 변경 / 신규 폴더 생성 | `docs/DIRECTORY_STRUCTURE.md` |
| 공통 Interface / Component 구조 변경 | `docs/ARCHITECTURE.md` |
| Experience Flow / Level Flow 변경 | `docs/ARCHITECTURE.md` |
| PlayerPhone 확장 방식 변경 | `docs/ARCHITECTURE.md` |
| Damage / Character 구조 변경 | `docs/ARCHITECTURE.md` |
| Feature 간 Dependency 변경 | `docs/ARCHITECTURE.md`, `docs/COLLABORATION.md` |
| Game Feature Plugin 신규 생성 | `docs/DIRECTORY_STRUCTURE.md`, `docs/ARCHITECTURE.md`, 해당 `docs/<Feature>/STATUS.md` |
| 브랜치 정책 확정 | `docs/COLLABORATION.md` §2 |
| 기능 구현 상태 변화 (`Planned` → `Partial` → `Implemented`) | `docs/ARCHITECTURE.md` §8, `docs/<Feature>/STATUS.md` |

### 문서 작성 원칙

* **구현하지 않은 것을 구현했다고 쓰지 않는다.**
* **테스트하지 않은 것을 테스트했다고 쓰지 않는다.**
* 불확실한 내용은 `TODO`, `Needs Verification`, `Planned`로 명시한다.
* 문서와 실제 구현이 다르면 **실제 구현을 우선 기록**하고 차이를 남긴다.
* 문서 변경은 관련 코드 변경과 **같은 커밋 또는 같은 PR**에 포함한다.

---

## 11. 작업 완료 기록 규칙

### 작업 시작 시

1. `docs/<Feature>/completed/`를 확인해 이미 구현된 기능인지 본다.
2. `docs/<Feature>/plans/`를 확인해 진행 중인 계획이 있는지 본다.
3. 복수 파일 수정 또는 신규 시스템 추가라면 `docs/<Feature>/plans/`에 계획 문서를 만든다.

계획 문서 형식 (`CLAUDE.md` 4절):

```text
# 목적
# 현재 상태
# 구현 범위
# 변경 예정 파일
# 구현 단계
# 다른 Feature에 미치는 영향
# 검증 방법
```

### 작업 완료 시

중요한 작업은 `docs/<Feature>/completed/`에 기록한다.

완료 문서 형식 (`CLAUDE.md` 15절):

```text
# 작업
# 구현 내용
# 변경 파일
# 주요 결정 사항
# 테스트 결과
# 남은 문제
```

관련 `plans/` 문서가 있으면 완료 상태로 갱신한다.

### 파일 명명 권장

```text
docs/<Feature>/plans/YYYY-MM-DD_<작업명>.md
docs/<Feature>/completed/YYYY-MM-DD_<작업명>.md
docs/<Feature>/specs/<기능명>.md
```

사소한 수정에는 문서를 만들지 않는다. 불필요한 빈 문서를 대량 생성하지 않는다.

---

## 12. 착수 전 팀이 결정해야 할 사항

아래는 **여러 사람이 동시에 작업을 시작하기 전에** 합의되어야 충돌을 막을 수 있는 항목이다.

### 해결됨 (2026-08-12)

| # | 항목 | 결과 |
|---|---|---|
| 1 | 계층 골격 디렉토리 생성 | **완료.** `Content/Core`, `Content/Gameplay`, `Content/Maps`, `Plugins/GameFeatures` |
| 2 | Git LFS 도입 | **완료.** §6.5 참조 (과거 이력 마이그레이션은 미실행) |
| 3 | `.gitignore`에 `*.slnx` 추가 | **완료.** ThirdParty 바이너리 예외 규칙도 함께 추가 |
| 4 | 타깃 기기 확정 | **Android 스탠드얼론.** 개발 중에는 PC |
| 5 | 음성 인식 방식 | **온디바이스 + 외부 서드파티 모듈 임포트** |
| 6 | Game Feature Plugin 사용 여부 | **사용 확정.** `.uplugin` 4개 생성 + `.uproject` 활성화 완료 |
| 7 | LFS 과거 이력 마이그레이션 | **실행 완료.** 전 브랜치 force push됨 — §6.5 참조 |

### 미해결

| # | 항목 | 이유 |
|---|---|---|
| 1 | **`UGameFeatureData` 에셋 4개 생성** | 에디터 작업. 없으면 Game Feature가 동작하지 않는다. `Plugins/GameFeatures/README.md` 참조 |
| 2 | `L_Main` 및 체험 Level 4종 생성 | **사용자가 직접 생성 예정.** 생성 전까지 템플릿 맵 공동 편집으로 인한 `.umap` 충돌 위험이 남는다 |
| 3 | Core / Shared 시스템별 담당자 지정 | 공통 클래스 동시 수정 방지. 특히 **Shared 전투 시스템은 담당자 1명 지정 필수** |
| 4 | PR 규칙 및 `develop` → `main` 승격 기준 확정 | 통합 브랜치가 `develop`이라는 점은 확정됨. 나머지 세부 규칙은 §2 참조 |
| 5 | 음성 인식 서드파티 모듈 선정 | arm64-v8a 지원 · 한국어 정확도 · 라이선스 · 오프라인 모델 크기 기준 평가 |
| 6 | Android 성능 예산 정의 | 동시 적 수 / 동시 투사체 수 / 드로우콜 상한 |
| 7 | `r.RayTracing` / `r.Substrate` 정리 | Android 타깃에 부적절한 설정 |
