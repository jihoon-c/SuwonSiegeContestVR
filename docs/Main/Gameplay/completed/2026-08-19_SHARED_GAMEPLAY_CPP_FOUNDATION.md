# 작업

Shared Gameplay C++ 전투·AI·최적화 기반 구현

# 구현 내용

- 공통 Character, Faction, Health, Damage, Projectile 계약
- 적 AI LOD, 단순 이동, 행동 상태, BT/StateTree 전환 훅
- Actor Pool과 재사용 Interface
- 공격자 위협 추적, 적대 표적 검색, 주기 공격 Component
- `GF_OngseongCrossbow` Runtime 모듈 및 총통/포탄 C++ Actor

# 주요 결정

- Android VR에서 원거리 적은 비표시·저빈도 단순 이동, 근거리 적만 고정밀 AI를 수행한다.
- 총통 표적 우선순위는 공격자, 성문 근접 적, 무작위 적이다.
- Shared Gameplay는 특정 Game Feature를 참조하지 않는다.

# 검증 결과

- Shared Character/Combat/AI LOD 단계는 Editor 타깃 빌드에 성공했다.
- 총통 Combat 후속 변경은 Editor Live Coding으로 인해 최종 빌드가 보류됐다.
- `git diff --check`는 커밋 전 통과했다.

# 후속 작업

Editor Asset/Level 통합 및 Android 실기기 검증은 `../plans/2026-08-19_EDITOR_INTEGRATION_AND_VALIDATION.md`를 따른다.
