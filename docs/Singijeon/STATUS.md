# Singijeon (신기전) — 현재 상태

**조사 기준일**: 2026-08-11 · **조사 기준 커밋**: `57dd875`
**계층**: Game Feature — `GF_Singijeon` (+ Shared Gameplay 의존)
**전체 상태**: `Status: Planned` — **구현된 것이 없다. Game Feature Plugin 자체가 존재하지 않는다.**

---

## 1. 담당 범위

* 신기전 (화차 / 발사대)
* 발사
* Target (표적)
* Damage
* 신기전 관련 PlayerPhone 기능
* 체험 완료 조건

### 계층 분류

| 요소 | 올바른 계층 | 사유 |
|---|---|---|
| 신기전 화차 / 발사대 | `GF_Singijeon` | 이 체험 전용 |
| 발사 연출 (다연장 일제사격 VFX/SFX) | `GF_Singijeon` | 전용 연출 |
| 신기전 로켓 고유 거동 | `GF_Singijeon` | 전용 |
| 표적 배치 / 체험 완료 조건 | `GF_Singijeon` | 전용 |
| Phone 기능 | `GF_Singijeon/Content/Phone/` | 확장 Component |
| **Projectile 기반 클래스** | **Shared Gameplay** | 쇠뇌 볼트와 공유 |
| **Damage / Health / Faction** | **Shared Gameplay** | 전투 있는 모든 체험이 공유 |
| **Enemy Soldier (표적이 병사인 경우)** | **Shared Gameplay** | 웅성/쇠뇌와 공유 |

> Damage는 **이 Feature의 것이 아니다.** `docs/OngseongCrossbow/STATUS.md`와 **같은 Shared Damage 시스템**을 사용한다.
> 두 Feature가 각자 Damage를 만들면 중복 구현이며 `CLAUDE.md` 17절 위반이다.

---

## 2. 현재 구현 상태 (실제 조사 결과)

| 요소 | 상태 | 실제 확인 내용 |
|---|---|---|
| `GF_Singijeon` 플러그인 | `Planned` | `Plugins/` 디렉토리 자체가 없다 |
| `L_Singijeon` Level | `Planned` | 존재하지 않음 |
| 신기전 화차 / 발사대 | `Planned` | 없음 |
| 발사 로직 | `Planned` | 없음 |
| 신기전 로켓 | `Planned` | 없음 |
| Target | `Planned` | 없음. 템플릿 `BP_WobbleTarget`은 물리 반응 샘플이며 데미지/점수 개념 없음 |
| Damage (Shared) | `Planned` | 없음 |
| Projectile 기반 (Shared) | `Planned` | 템플릿 `BP_Projectile`은 **데미지 로직이 없는** 시각 샘플 |
| `BP_SingijeonExperienceManager` | `Planned` | 없음 |
| Phone 확장 Component | `Planned` | PlayerPhone 본체부터 없음 |
| 체험 완료 조건 | `Planned` | 없음 |

**활용 가능한 기존 자산**

* `BP_Pistol` + `BP_Projectile` — "발사 → 투사체 생성" 흐름의 참고 구조
* `LevelPrototyping/Interactable/Target/BP_WobbleTarget` — 표적 프로토타이핑용 (물리 흔들림만 구현됨)
* `XRFramework/VFX/` Niagara 시스템 — 발사 이펙트 참고

---

## 3. 설계 결정 필요 사항 (TODO)

### 3.1 발사 방식

신기전은 **다연장 로켓**이다. 한 발씩이 아니라 여러 발이 동시/연속 발사되는 것이 특징이다.

* 조작: 심지에 불 붙이기 vs 레버 당기기 vs 버튼
  → 교육 콘텐츠라면 "화약 심지에 점화" 연출이 학습 효과가 크다 (`Needs Verification`)
* 발사 형태: 일제 발사 vs 순차 연속 발사
* 발사 전 **각도 조절** 포함 여부 — 포물선 조준이 게임플레이의 핵심이 될 수 있다
* 발사 횟수 / 재장전 개념

### 3.2 투사체 거동

* 직선 vs 포물선(중력 적용) vs 로켓 추진 후 낙하
  → 신기전 특성상 **추진 + 포물선**이 사실적이나, 조준 난이도가 올라간다
* 명중 판정: 투사체 충돌 vs 폭발 반경(AoE)
* 성능: 다연장이므로 동시 투사체 수가 많다. **VR 스탠드얼론에서 Niagara + 물리 부하 주의**

### 3.3 Target

* 표적의 정체: 고정 과녁 / 적 병사 / 구조물
  → **적 병사라면 Shared Enemy Soldier를 사용**하고 이 Feature에 새로 만들지 않는다
* 점수/명중 판정 표시 방식
* 표적 수와 배치

### 3.4 Damage (Shared — 영향 범위 큼)

* `GF_OngseongCrossbow`와 **동일한 Damage / Faction 구조를 공유**해야 한다
* 폭발형 데미지(반경 감쇠)가 필요하면 Shared Damage 시스템 설계에 **처음부터 포함**시켜야 한다
* 구체 클래스 검사 금지, Faction + Damage Policy 기반 (`CLAUDE.md` 9절)

### 3.5 플레이어 이동

* 발사대 앞 고정 위치 권장 (다연장 조작 특성상)
* 이동 제한 시 IMC 전환 필요

### 3.6 체험 완료 조건

* 예: 표적 N개 명중 / 지정 발사 횟수 완료 / 제한 시간 내 목표 달성
* 실패 조건 존재 여부 (교육 콘텐츠이므로 재시도 권장, 확인 필요)
* 완료 보고 방식 → **Main의 `ExperienceSubsystem` 인터페이스 확정 대기**

### 3.7 PlayerPhone 기능

* 명중 수 / 남은 표적 표시? 신기전 원리 설명? 조준 각도 표시?

---

## 4. 선행 조건

1. Game Feature Plugin 사용 여부 확정
2. `ExperienceSubsystem` 인터페이스 확정 (Main)
3. **Shared Projectile 기반 클래스 확정** — `GF_OngseongCrossbow`와 공유
4. **Shared Damage / Faction 구조 확정** — `GF_OngseongCrossbow`와 공유
5. VR Player Pawn 및 상호작용 방식 확정 (Core)

---

## 5. 주의사항 · 위험 요소

* **`GF_OngseongCrossbow`와 Shared 자산이 크게 겹친다.** 두 Feature를 다른 개발자가 동시에 진행하면
  Projectile / Damage / Faction을 **각자 만들어 중복 구현할 위험이 매우 높다.**
  → 착수 전에 Shared 전투 시스템의 담당자를 **한 명으로 지정**하고 인터페이스를 먼저 확정한다.
* 다연장 발사는 동시 투사체·VFX 수가 많아 **성능 리스크가 이 프로젝트에서 가장 크다.**
  타깃 기기(특히 Quest/PICO 스탠드얼론) 확정 후 동시 로켓 수 상한을 미리 정할 것.
* 폭발 반경 데미지가 필요하면 Shared Damage 설계 **초기에** 반영해야 한다. 나중에 넣으면 인터페이스가 흔들린다.
