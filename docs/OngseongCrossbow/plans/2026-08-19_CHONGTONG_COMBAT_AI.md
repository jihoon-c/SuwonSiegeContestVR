# 목적

웅성/총통 시나리오의 고정 총통이 Shared Combat Damage 계약으로 적을 공격하도록 구현한다.

# 현재 상태

Shared Enemy는 원거리 AI LOD와 단순 이동 기반을 갖지만, 공격자 위협 기록·근접 공격·아군 표적 우선순위는 없다. `GF_OngseongCrossbow`는 콘텐츠 플러그인만 있고 C++ 모듈이 없다.

# 구현 범위

- Shared: 공격자 위협 추적, 진영 기반 표적 검색, 거리 내 주기 공격 Component
- Feature: 총통과 총통 포탄 Actor
- 총통 표적 우선순위: 총통 공격자 → 성문과 가장 가까운 적 → 무작위 적

# 구현 단계

1. Shared Damage 후속 Component를 추가한다.
2. 웅성/총통 플러그인 Runtime 모듈을 추가한다.
3. 총통 Actor가 우선순위에 따라 투사체 Damage Spec을 발사하게 한다.
4. Editor 빌드로 검증한다.

# 다른 Feature에 미치는 영향

Shared Component는 Feature를 참조하지 않는다. 총통과 포탄은 `GF_OngseongCrossbow`에만 존재한다.

# 검증 방법

- Unreal Editor 타깃 빌드
- Feature → Shared 단방향 모듈 의존성 확인

# 진행 상태 (2026-08-19)

구현은 완료했다. Shared에는 `UCombatThreatComponent`, `UCombatTargetingComponent`,
`UCombatAttackComponent`를 추가했고, 적 기반 Character는 `Advance → Assault`와
`Retreat` 전환 API를 제공한다. Feature에는 `AChongtongCannonActor`와
`AChongtongProjectileActor` Runtime 모듈을 추가했다.

Editor가 Live Coding 상태여서 UnrealBuildTool의 최종 컴파일 검증은 보류됐다.
에디터에서 Live Coding을 종료한 뒤 이 계획의 Build 명령을 실행해야 한다.
