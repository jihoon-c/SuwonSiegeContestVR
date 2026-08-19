# Gongsimdon (공심돈) — 현재 상태

**조사 기준일**: 2026-08-11 · **조사 기준 커밋**: `57dd875`
**계층**: Game Feature — `GF_Gongsimdon` (+ Shared Gameplay 의존)
**전체 상태**: `Status: Planned` — **구현된 것이 없다. Game Feature Plugin 자체가 존재하지 않는다.**

---

## 1. 담당 범위

* 공심돈 구조물
* 침입하는 적 탐색
* 탐지 로직
* 공심돈 관련 PlayerPhone 기능
* 체험 완료 조건

### 계층 분류

| 요소 | 올바른 계층 | 사유 |
|---|---|---|
| 공심돈 구조물 | `GF_Gongsimdon` | 이 체험 전용 |
| 탐지/관측 로직 (망원 시야, 표시) | `GF_Gongsimdon` | 이 체험 전용 게임플레이 |
| 탐지 UI | `GF_Gongsimdon/Content/UI/` | 전용 |
| Phone 기능 | `GF_Gongsimdon/Content/Phone/` | 확장 Component |
| **침입하는 적 (Enemy Soldier)** | **Shared Gameplay** | 웅성/쇠뇌 체험과 공유 |
| **적 AI / 이동** | **Shared Gameplay** | 재사용 대상 |
| **Faction** | **Shared Gameplay** | 적/아군 구분 공통 |

> 적 병사를 이 Feature 안에 만들지 않는다. `GF_OngseongCrossbow`와 **같은 Shared Enemy를 사용**한다.

---

## 2. 현재 구현 상태 (실제 조사 결과)

| 요소 | 상태 | 실제 확인 내용 |
|---|---|---|
| `GF_Gongsimdon` 플러그인 | `Planned` | `Plugins/` 디렉토리 자체가 없다 |
| `L_Gongsimdon` Level | `Planned` | 존재하지 않음 |
| 공심돈 구조물 | `Planned` | 없음 |
| 침입 적 | `Implemented (C++ base)` | Shared Enemy, AI LOD, 단순 이동 기반. Feature 메시/배치/도주 목표는 미구현 |
| 탐지 로직 | `Planned` | 없음 |
| `BP_GongsimdonExperienceManager` | `Planned` | 없음 |
| 탐지 UI | `Planned` | 없음 |
| Phone 확장 Component | `Planned` | PlayerPhone 본체부터 없음 |
| 체험 완료 조건 | `Planned` | 없음 |

**활용 가능한 기존 자산**: `BP_XRPawn`의 `WidgetInteraction` 및 MotionController Aim은 "가리켜서 표시" 방식 탐지의 기반으로 쓸 수 있다.

---

## 3. 배경 정보 (설계 참고)

공심돈(空心墩)은 속이 빈 망루형 구조물로, 내부에서 적을 관측하고 총안(銃眼)을 통해 밖을 살피는 시설이다.
따라서 이 체험은 **전투보다 "관측·탐지"에 초점**이 맞는 것이 콘텐츠 의도와 부합한다. (`Needs Verification` — 기획 확인 필요)

---

## 4. 설계 결정 필요 사항 (TODO)

### 4.1 탐지 방식 — **이 체험의 핵심이자 가장 불명확한 부분**

"침입하는 적 탐색"의 게임플레이가 구체적으로 정해지지 않았다. 후보:

* 후보 A: **총안(구멍)으로 밖을 보며** 시야 범위 내 적을 눈으로 찾고 가리켜 표시
* 후보 B: 여러 방향을 **순찰 관측**하며 정해진 시간 내 모든 적 발견
* 후보 C: 은폐/엄폐한 적을 **주의 깊게 관찰**해 찾아내기 (숨은그림찾기형)
* 후보 D: 망원 도구로 원거리 적 식별

**확인 필요**: 교육적으로 전달하려는 것이 "공심돈의 구조적 기능(속이 비어 사방 관측 가능)"인지, 단순 탐색 재미인지.

### 4.2 탐지 판정

* 판정 방식: 시선(카메라) 기반 vs 컨트롤러 Aim 기반 vs 클릭 선택
* 판정 조건: 일정 시간 응시 / 조준 후 트리거 / 근접
* 오탐(잘못 지목) 처리 방식
* 이미 탐지한 적의 표시 유지 여부

### 4.3 적 배치와 이동

* 적이 정적으로 배치되어 있는지, 이동 중인지
* 이동한다면 경로 방식 (NavMesh vs 스플라인)
* 적 수 및 난이도

### 4.4 플레이어 이동

* 공심돈 내부 고정 위치 vs 층간 이동 vs 관측 지점 간 텔레포트
* 내부 공간이라 이동 범위가 좁을 것으로 예상 → 고정 또는 지점 이동 권장 (`Needs Verification`)

### 4.5 체험 완료 조건

* 예: 침입한 적 N명 전부 탐지 / 제한 시간 내 탐지
* 제한 시간 유무
* 실패 조건 존재 여부
* 완료 보고 방식 → **Main의 `ExperienceSubsystem` 인터페이스 확정 대기**

### 4.6 PlayerPhone 기능

* 탐지한 적 수 / 남은 적 수 표시? 공심돈 구조 설명? 탐지 힌트?

---

## 5. 선행 조건

1. **탐지 게임플레이 정의 (기획 확정)** — 가장 시급
2. Game Feature Plugin 사용 여부 확정
3. `ExperienceSubsystem` 인터페이스 확정 (Main)
4. 탐지 시 도주 경로와 목표점 정의
   → Shared Enemy의 Behavior State를 `Retreat`로 바꾸고 단순 이동 목표를 반대편 탈출점으로 지정한다.
5. VR Player Pawn 및 상호작용 방식 확정 (Core)

---

## 6. 주의사항

* 이 체험은 전투(데미지)가 없을 가능성이 높다. 그렇다면 **Health / Damage 시스템에는 의존하지 않고 Enemy + Faction + AI만 사용**한다. (`Needs Verification`)
* 탐지 로직을 Shared Gameplay에 만들 필요는 없다. 다른 체험에서 "탐지"를 재사용할 계획이 없다면 이 Feature에 둔다.
* 공심돈 내부는 좁은 실내이므로 VR 카메라 클리핑·오클루전·조명 처리를 초기에 확인하는 것이 좋다.
