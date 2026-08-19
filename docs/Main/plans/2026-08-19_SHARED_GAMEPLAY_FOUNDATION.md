# 목적

여러 체험이 공유할 수 있는 Gameplay 전투 기반을 C++로 제공한다.

# 현재 상태

`Gameplay` Shared 계층에는 Character, Health, Damage, Faction, Projectile 구현이 없다. 신기전 플러그인에는 체험 전용 투사체가 존재하지만 공통 데미지·진영 규약을 사용하지 않는다.

# 구현 범위

- 진영 Component와 적대 판정
- 체력 Component와 피격/사망 이벤트
- 데이터 기반 데미지 명세, 데미지 수신 Interface, 데미지 적용 Utility
- Combat Character 및 Enemy/Ally 파생 기반 클래스
- 충돌·수명·데미지 적용을 갖는 공통 Projectile Actor

AI 행동, 스폰/웨이브, UI 표현, 밸런스 수치, Game Feature 전용 클래스의 마이그레이션은 포함하지 않는다.

# 변경 예정 파일

- `Source/SuwonSiegeContestVR/Public/Gameplay/**`
- `Source/SuwonSiegeContestVR/Private/Gameplay/**`
- `docs/ARCHITECTURE.md`
- `docs/Main/completed/`

# 구현 단계

1. Combat 데이터와 Component/Interface 계약을 정의한다.
2. Character와 Projectile 기반 Actor를 구현한다.
3. 컴파일 가능한 모듈 의존성과 코드를 검증한다.
4. 구현 상태와 미결정 기획 항목을 문서화한다.

# 다른 Feature에 미치는 영향

Shared Gameplay는 Core 또는 Game Feature를 참조하지 않는다. 각 Game Feature는 이후 이 기반 클래스 또는 Component를 선택적으로 사용한다. 기존 신기전 클래스의 동작은 변경하지 않는다.

# 검증 방법

- Unreal Build Tool로 프로젝트 Editor 타깃을 빌드한다.
- 정적 확인으로 Shared Gameplay → Core/Game Feature 역의존이 없는지 확인한다.
