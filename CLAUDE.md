
# Hwaseong VR Development Rules

이 문서는 프로젝트에서 작업하는 모든 AI Coding Agent가 반드시 따라야 하는 최상위 작업 규칙이다.

본 프로젝트는 Unreal Engine 5.8 기반 VR 교육 콘텐츠이며 여러 개발자가 동시에 기능을 개발한다.

AI Agent는 코드 또는 에셋을 수정하기 전에 반드시 관련 문서와 기존 구현을 먼저 조사해야 한다.

---

# 1. 기본 원칙

## 1.1 문서 우선 개발

작업 요청을 받으면 즉시 구현을 시작하지 않는다.

반드시 다음 순서를 따른다.

1. 사용자 요청 분석
2. 작업에 해당하는 Feature 식별
3. 관련 문서 탐색
4. 기존 작업 진행 상황 확인
5. 관련 코드 및 에셋 탐색
6. 구현 계획 작성 또는 기존 계획 확인
7. 영향 범위 확인
8. 구현
9. 검증
10. 작업 완료 문서 업데이트

문서와 실제 구현이 충돌하는 경우 실제 구현 상태를 확인한 뒤 사용자에게 차이를 알리고, 임의로 대규모 구조 변경을 수행하지 않는다.

---

# 2. 프로젝트 문서 위치

모든 프로젝트 문서는 다음 위치에 존재한다.

`/docs`

최상위 공통 문서:

* `docs/DIRECTORY_STRUCTURE.md`
* `docs/ARCHITECTURE.md`
* `docs/COLLABORATION.md`

Feature별 문서:

* `docs/Main/`
* `docs/Geojunggi/`
* `docs/OngseongCrossbow/`
* `docs/Gongsimdon/`
* `docs/Singijeon/`

각 Feature 디렉토리는 기본적으로 다음 구조를 가진다.

```text
FeatureName/
├─ plans/
├─ completed/
└─ specs/
```

### plans/

구현 예정 또는 현재 진행 중인 작업 계획을 저장한다.

작업을 시작하기 전에 동일한 기능에 대한 기존 계획이 존재하는지 반드시 확인한다.

### completed/

완료된 작업 기록을 저장한다.

이미 구현된 기능을 중복 구현하지 않도록 작업 전에 반드시 확인한다.

### specs/

기능의 설계 및 요구사항 정의서를 저장한다.

기능 구현 시 가장 우선적으로 참고해야 하는 문서이다.

---

# 3. 작업 시작 절차

사용자의 요청을 받으면 다음 절차를 따른다.

## Step 1. Feature 식별

요청이 다음 중 어디에 해당하는지 판단한다.

* Main
* Geojunggi
* OngseongCrossbow
* Gongsimdon
* Singijeon
* Shared/Core Gameplay

여러 Feature에 영향을 미치는 경우 이를 명확하게 구분한다.

---

## Step 2. 관련 문서 탐색

관련 Feature의 다음 디렉토리를 확인한다.

```text
docs/<Feature>/specs/
docs/<Feature>/plans/
docs/<Feature>/completed/
```

공통 시스템과 관련된 작업이라면 추가로 다음을 확인한다.

```text
docs/ARCHITECTURE.md
docs/DIRECTORY_STRUCTURE.md
docs/COLLABORATION.md
```

관련성이 높은 문서를 먼저 읽고 작업 상태를 파악한다.

---

## Step 3. 기존 작업 상태 확인

다음을 확인한다.

* 이미 구현된 기능인가?
* 구현 중인 기능인가?
* 다른 개발자가 담당하고 있는 기능인가?
* 관련 계획 문서가 존재하는가?
* 기존 설계와 충돌하는가?
* 공통 시스템에 영향을 미치는가?

`completed/` 기록을 반드시 확인하여 이미 완료된 기능을 다시 구현하지 않는다.

---

## Step 4. 코드 및 에셋 탐색

문서 확인 후 실제 프로젝트 구조를 탐색한다.

관련 Blueprint, C++ 클래스, Data Asset, Widget, Actor Component, Level 등을 확인한다.

파일 이름만 보고 기능을 추측하지 않는다.

가능하면 참조 관계와 실제 구현을 확인한다.

---

# 4. 구현 계획 규칙

복수 파일 수정 또는 새로운 시스템 추가가 필요한 경우 구현 전에 계획을 작성한다.

기존 plan 문서가 있다면 이를 업데이트하고, 없다면 필요한 경우 생성한다.

Plan에는 최소한 다음 항목이 포함되어야 한다.

```text
# 목적

# 현재 상태

# 구현 범위

# 변경 예정 파일

# 구현 단계

# 다른 Feature에 미치는 영향

# 검증 방법
```

사소한 수정에는 불필요한 문서를 생성하지 않는다.

---

# 5. 프로젝트 Architecture 규칙

프로젝트는 크게 다음 세 계층으로 나뉜다.

```text
Core
Shared Gameplay
Game Features
```

## Core

프로젝트 전체 실행을 위한 기반 시스템이다.

예:

* VR Pawn
* VR Input
* Interaction
* PlayerPhone
* ExperienceSubsystem
* Quiz System
* Voice Recognition
* 공통 Interface

---

## Shared Gameplay

둘 이상의 체험에서 재사용 가능한 Gameplay 기능이다.

예:

* Enemy Soldier
* Ally Soldier
* Combat Character
* Health
* Damage
* Faction
* Projectile
* AI
* 공통 UI Widget
* Gameplay Tags

특정 체험에서 사용된다는 이유만으로 Shared Gameplay 기능을 Game Feature 내부로 이동하지 않는다.

---

## Game Features

특정 체험에서만 필요한 기능이다.

현재 Game Feature는 다음 네 개이다.

```text
GF_Geojunggi
GF_OngseongCrossbow
GF_Gongsimdon
GF_Singijeon
```

예를 들어 적 병사는 여러 체험에서 사용할 수 있으므로 Shared Gameplay이다.

반면 쇠뇌나 충차가 웅성/쇠뇌 체험에서만 사용된다면 `GF_OngseongCrossbow`에 속한다.

---

# 6. Dependency 규칙

의존성은 원칙적으로 다음 방향만 허용한다.

```text
Game Feature
     ↓
Shared Gameplay
     ↓
Core
```

반대 방향 의존성을 만들지 않는다.

금지 예:

```text
Core → GF_OngseongCrossbow
EnemySoldier → GF_Singijeon
CommonWidget → GF_Geojunggi
```

허용 예:

```text
GF_Singijeon → EnemySoldier
GF_OngseongCrossbow → Damage System
GF_Gongsimdon → Common UI
```

Core 또는 Shared Gameplay 클래스에서 특정 Game Feature를 직접 참조해야 하는 상황이 생기면 Interface, Event, Gameplay Tag, Component 또는 Data Asset 기반으로 의존성을 역전하는 방법을 우선 검토한다.

---

# 7. Level Blueprint 규칙

Level Blueprint에는 핵심 Gameplay Logic을 작성하지 않는다.

Level Blueprint는 가능한 한 다음 역할만 수행한다.

* Level 초기화
* Level 배치 객체 연결
* 간단한 Level Event 전달

게임 진행 로직은 별도의 Manager 또는 Component에 구현한다.

예:

```text
BP_CrossbowExperienceManager
BP_GeojunggiExperienceManager
BP_GongsimdonExperienceManager
BP_SingijeonExperienceManager
```

---

# 8. PlayerPhone 규칙

PlayerPhone 자체는 Core 시스템이다.

특정 체험에만 필요한 기능을 PlayerPhone 본체에 직접 추가하지 않는다.

체험별 Phone 기능은 Game Feature가 제공하는 Component 또는 Extension 형태로 구현한다.

예:

```text
BP_PlayerPhone

+ GeojunggiPhoneComponent
+ CrossbowPhoneComponent
+ GongsimdonPhoneComponent
+ SingijeonPhoneComponent
```

필요한 Feature가 활성화되었을 때 해당 기능을 활성화하거나 주입한다.

---

# 9. Character 및 Combat 규칙

공통 Character 계층을 사용한다.

예:

```text
CombatCharacter
├─ EnemySoldier
└─ AllySoldier
```

Character가 필요한 공통 Gameplay 기능은 Component를 우선 사용한다.

예:

```text
HealthComponent
FactionComponent
```

특정 공격 시스템에서 구체적인 Character Class를 직접 검사하지 않는다.

금지:

```text
If Actor Is BP_EnemySoldier
    Apply Damage
```

권장:

```text
Check Faction
Check Damage Policy
Apply Damage
```

공격 시스템과 대상 시스템을 느슨하게 결합한다.

---

# 10. UI 규칙

둘 이상의 Feature에서 사용할 수 있는 Widget은 Shared Gameplay UI에 배치한다.

예:

* 공통 Progress UI
* Timer
* 안내 Popup
* Interaction Prompt
* 체력 UI
* VR World Space UI

특정 체험에서만 사용하는 Widget은 해당 Game Feature에 배치한다.

예:

```text
GF_OngseongCrossbow/UI/WBP_CrossbowAmmo
```

공통 Widget이 특정 Game Feature를 직접 참조하지 않도록 한다.

---

# 11. Gameplay Ability System 규칙

현재 프로젝트는 Gameplay Ability System을 기본 전투 프레임워크로 사용하지 않는다.

현재 기본 전투 구조는 다음을 우선한다.

* Health Component
* Faction Component
* Damage Interface/System
* Gameplay Tags

단순 공격이나 체험 장비 기능을 Gameplay Ability로 만들지 않는다.

다음과 같은 요구사항이 발생할 경우 GAS 도입을 다시 검토한다.

* 복잡한 Attribute 시스템
* Buff / Debuff
* Cooldown
* Resource Cost
* 지속 효과
* 다단계 Ability
* 네트워크 Gameplay
* 다수의 Character Ability

GAS 도입은 프로젝트 전역 Architecture 변경이므로 사용자 요청 없이 임의로 도입하지 않는다.

---

# 12. Blueprint / C++ 선택 기준

공통 기반 시스템과 여러 Feature에서 사용하는 시스템은 가능하면 C++ 또는 안정적인 공통 Component로 작성한다.

예:

* ExperienceSubsystem
* Health Component
* Damage Interface
* Faction Component
* 공통 Utility

체험별 연출과 빠르게 변경될 Gameplay는 Blueprint 중심으로 작성할 수 있다.

예:

* Enemy Wave
* 거중기 체험 순서
* 쇠뇌 연출
* 신기전 발사 이벤트
* 공심돈 탐색 이벤트

불필요하게 모든 기능을 C++로 변환하지 않는다.

---

# 13. Naming 규칙

Unreal Asset은 역할을 명확하게 나타내는 Prefix를 사용한다.

예:

```text
BP_     Blueprint Actor
WBP_    Widget Blueprint
BPC_    Blueprint Actor Component
DA_     Data Asset
DT_     Data Table
MI_     Material Instance
M_      Material
T_      Texture
SM_     Static Mesh
SK_     Skeletal Mesh
ABP_    Animation Blueprint
L_      Level
SFX_    Sound Effect
```

Game Feature:

```text
GF_Geojunggi
GF_OngseongCrossbow
GF_Gongsimdon
GF_Singijeon
```

---

# 14. 협업 규칙

이 프로젝트는 여러 개발자가 동시에 수정한다.

따라서 다음을 반드시 지킨다.

다른 개발자의 작업을 임의로 삭제하거나 대규모로 수정하지 않는다.

레벨에 이미 배치된 Actor의 위치, 회전, 스케일은 사용자가 명시적으로 요청한 경우에만 수정한다. 버그 점검, 에셋 교체, 컴포넌트 수정, 자동화 스크립트 실행 중에는 배치 Transform을 변경하거나 재저장하지 않는다.

관련 문서에서 담당자 또는 진행 중인 작업이 확인되면 이를 존중한다.

자신의 작업 범위를 넘어서는 변경이 필요한 경우 먼저 영향 범위를 확인한다.

불필요한 Blueprint 재저장, Asset 이동, 폴더 Rename을 피한다.

공통 Blueprint나 공통 C++ 클래스를 수정할 경우 이를 사용하는 모든 Feature에 미치는 영향을 확인한다.

---

# 15. 작업 완료 절차

구현이 끝났다고 즉시 작업을 종료하지 않는다.

다음을 수행한다.

1. 구현 결과 확인
2. Compile Error 확인
3. Blueprint Error 확인
4. 참조 오류 확인
5. 영향받는 Feature 확인
6. 기존 계획과 구현 결과 비교
7. 관련 문서 업데이트

완료된 중요한 작업은 해당 Feature의:

```text
docs/<Feature>/completed/
```

에 기록한다.

완료 문서는 최소한 다음 내용을 기록한다.

```text
# 작업

# 구현 내용

# 변경 파일

# 주요 결정 사항

# 테스트 결과

# 남은 문제
```

관련 Plan이 존재한다면 완료 상태로 업데이트한다.

---

# 16. 문서와 코드의 동기화

코드를 수정하여 Architecture 또는 기능 동작이 달라졌다면 관련 문서를 함께 수정한다.

특히 다음 사항을 변경했다면 반드시 문서 수정 필요 여부를 검토한다.

* Directory 구조
* 공통 Interface
* Component 구조
* Experience Flow
* PlayerPhone 확장 방식
* Damage 시스템
* Character 구조
* Feature 간 Dependency
* Level Flow

코드와 문서를 서로 다른 상태로 방치하지 않는다.

---

# 17. 금지 사항

다음을 하지 않는다.

* 관련 문서를 읽지 않고 바로 구현
* 이미 존재하는 기능 중복 구현
* Game Feature가 Core에 역의존하도록 구현
* 특정 Feature 로직을 공통 Character에 하드코딩
* Level Blueprint에 대규모 Gameplay Logic 구현
* 공통 UI가 특정 Feature를 직접 참조
* 사용자 요청 없이 대규모 Refactoring
* 사용자 요청 없이 GAS 도입
* 다른 개발자의 작업을 확인하지 않고 덮어쓰기
* 구현하지 않은 내용을 완료 문서에 기록
* 테스트하지 않은 내용을 테스트 완료라고 기록

---

# 18. Agent 작업 사고 순서

모든 작업에서 다음 질문을 순서대로 판단한다.

```text
무엇을 요청받았는가?
        ↓
어느 Feature인가?
        ↓
관련 문서는 무엇인가?
        ↓
이미 작업된 것이 있는가?
        ↓
누가 작업 중인가?
        ↓
관련 코드/에셋은 어디에 있는가?
        ↓
Core인가 Shared인가 Feature인가?
        ↓
어떤 의존성이 생기는가?
        ↓
최소 변경으로 구현 가능한가?
        ↓
구현
        ↓
검증
        ↓
문서 업데이트
```

이 절차는 기능 개발, 버그 수정, 리팩터링 모두에 적용한다.
