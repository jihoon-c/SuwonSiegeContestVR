# Geojunggi (거중기) — 현재 상태

**조사 기준일**: 2026-08-11 · **조사 기준 커밋**: `57dd875`
**계층**: Game Feature — `GF_Geojunggi`
**전체 상태**: `Status: Planned` — **구현된 것이 없다. Game Feature Plugin 자체가 존재하지 않는다.**

---

## 1. 담당 범위

* 거중기 조작 체험
* 성벽 건축 체험
* 거중기 관련 PlayerPhone 기능
* 체험 완료 조건

---

## 2. 현재 구현 상태 (실제 조사 결과)

| 요소 | 상태 | 실제 확인 내용 |
|---|---|---|
| `GF_Geojunggi` 플러그인 | `Planned` | `Plugins/` 디렉토리 자체가 없다 |
| `L_Geojunggi` Level | `Planned` | 존재하지 않음 |
| 거중기 Actor | `Planned` | 관련 Blueprint / 메시 없음 |
| 성벽 / 석재 Actor | `Planned` | 없음 |
| `BP_GeojunggiExperienceManager` | `Planned` | 없음 |
| Phone 확장 Component | `Planned` | PlayerPhone 본체부터 없음 |
| 전용 UI Widget | `Planned` | 없음 |
| 체험 완료 조건 | `Planned` | 없음 |

**활용 가능한 기존 자산**: 템플릿의 `BP_GrabComponent`(잡기), `BP_XRPawn`의 Grab/Teleport, `LevelPrototyping`의 프로토타입 메시(SM_Cube 등)로 초기 프로토타이핑은 가능하다.

---

## 3. 설계 결정 필요 사항 (TODO)

### 3.1 거중기 조작 방식

거중기는 도르래로 무거운 석재를 들어올리는 장치다. VR 조작 방식이 정해지지 않았다.

* 후보 A: 밧줄/손잡이를 **양손으로 잡고 당기기** — 몰입감 높음, 물리 구현 난이도 높음
* 후보 B: 레버/크랭크 **회전 조작** — 구현 난이도 중간, 조작 직관성 양호
* 후보 C: 버튼/트리거로 **상승·하강 제어** — 가장 단순, 학습 효과는 낮음

**확인 필요**: 교육 목적상 "도르래로 힘을 줄인다"는 원리를 체감시키는 것이 목표인지, 단순 조작 체험인지.

### 3.2 성벽 건축

* 석재를 몇 개 쌓아야 완료인지
* 배치 방식: 물리 시뮬레이션 vs 스냅(Snap) 배치
  → VR + 물리는 불안정하기 쉬우므로 **스냅 배치 권장**
* 배치 위치 가이드(고스트 표시) 제공 여부

### 3.3 플레이어 이동

* 고정 위치 vs 텔레포트 vs 지정 지점 이동
* 거중기 조작 중 이동 제한 필요 여부 (제한한다면 IMC 전환 필요 — `docs/ARCHITECTURE.md` §10 문제 4 참조)

### 3.4 체험 완료 조건

* 예: 석재 N개를 정해진 위치에 쌓으면 완료
* 제한 시간 유무
* 완료 시 `ExperienceSubsystem`에 보고하는 방식 → **Main의 인터페이스 확정 대기**

### 3.5 PlayerPhone 기능

* 거중기 체험에서 Phone이 무엇을 하는지 미정 (조작 안내? 진행도? 도르래 원리 설명?)
* PlayerPhone 본체가 없으므로 Main 선행 필요

---

## 4. 계층 분류 주의사항

| 만들 것 | 위치 |
|---|---|
| 거중기 Actor, 도르래, 밧줄 | `GF_Geojunggi` (이 체험 전용) |
| 성벽 석재, 배치 지점 | `GF_Geojunggi` |
| `BP_GeojunggiExperienceManager` | `GF_Geojunggi` |
| 거중기 전용 UI | `GF_Geojunggi/Content/UI/` |
| Phone 확장 Component | `GF_Geojunggi/Content/Phone/` |
| **잡기/상호작용 기반 로직** | **Core** — 이 Feature 안에 만들지 않는다 |
| **진행 표시 / Timer 등 공통 Widget** | **Shared Gameplay UI** |

이 체험에는 전투 요소가 없으므로 Enemy/Damage 계열 의존은 발생하지 않을 것으로 보인다. (`Needs Verification`)

---

## 5. 선행 조건

이 체험 구현을 시작하려면 다음이 먼저 필요하다.

1. Game Feature Plugin 사용 여부 확정 (팀 결정)
2. `ExperienceSubsystem` 인터페이스 확정 (Main)
3. VR Player Pawn 확정 (Core)
4. 상호작용(잡기/조작) 공통 인터페이스 확정 (Core)
5. PlayerPhone 확장 방식 확정 (Core)

**단, 위가 확정되기 전에도** 거중기 조작 방식 프로토타입은 템플릿 Grab 기능으로 독립 검증 가능하다.
프로토타입은 `Content/Developers/<이름>/`에서 진행하고, 확정 후 정식 위치로 옮긴다.
