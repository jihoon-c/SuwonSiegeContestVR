# 작업

총통(플레이어/아군) muzzle flash와 포탄 착탄 폭발 이펙트를 Niagara에서 Cascade Particle System으로 교체.

# 조사 결과

프로젝트에 사용 가능한 Cascade 폭발 계열 파티클이 존재한다 (`Content/StarterContent/Particles/`).

* `P_Explosion` — 폭발 (착탄/포구 화염)
* `P_Sparks` — 불꽃 (장전 성공 피드백)
* `P_Fire`, `P_Smoke`, `P_Steam_Lit`, `P_Ambient_Dust`

UE 5.8 에서도 `UParticleSystem` / `UGameplayStatics::SpawnEmitterAtLocation` 런타임 API는 유효하며,
`EPSCPoolMethod::AutoRelease` 로 Niagara 때와 동일하게 풀링된다. Engine 모듈 의존성만 필요하므로
Build.cs 변경은 없다.

# 구현 내용

* `UCombatFXLibrary::SpawnPooledEmitterAtLocation` 추가 (Cascade 버전, PSC 풀 사용).
  기존 Niagara용 `SpawnPooledSystemAtLocation` 은 다른 용도를 위해 그대로 유지.
* `AChongtongProjectileActor::ExplosionEffect` 타입을 `UNiagaraSystem*` → `UParticleSystem*`,
  기본값 `NS_Impact_Concrete` → `P_Explosion`.
* `AChongtongCannonActor::MuzzleEffect` / `LoadSuccessEffect` 타입 동일 변경,
  기본값 `NS_MuzzleFlash` → `P_Explosion`, `NS_Pickup_Success` → `P_Sparks`.
* `PlayFeedback` 시그니처를 `UParticleSystem*` 로 변경.

# 변경 파일

* `Source/SuwonSiegeContestVR/Public/Gameplay/Combat/CombatFXLibrary.h`
* `Source/SuwonSiegeContestVR/Private/Gameplay/Combat/CombatFXLibrary.cpp`
* `Plugins/.../Public/Ongseong/ChongtongProjectileActor.h`
* `Plugins/.../Private/Ongseong/ChongtongProjectileActor.cpp`
* `Plugins/.../Public/Ongseong/ChongtongCannonActor.h`
* `Plugins/.../Private/Ongseong/ChongtongCannonActor.cpp`

# 주요 결정 사항

* Niagara 경로를 삭제하지 않고 Cascade 헬퍼를 병렬 추가했다. `InteractionHighlightComponent` 등
  다른 Core 기능이 여전히 Niagara를 사용한다.
* `bPreCullCheck` 인자는 Cascade에 대응 개념이 없어 제거했다.

# 테스트 결과

* 코드 레벨 교체 및 잔여 Niagara 참조 제거 확인.
* 에디터 컴파일/플레이 검증은 아직 수행하지 않음.

# 남은 문제

* `BP_AllyChongtong` 의 Class Defaults 에 Niagara `SimpleExplosion` MuzzleEffect 오버라이드가
  남아 있다. 프로퍼티 타입이 바뀌었으므로 로드 시 무시되고 C++ 기본값(P_Explosion)이 적용되지만,
  에디터에서 한 번 열어 재저장해 스테일 엔트리를 정리하는 것이 좋다.
  `BP_ChongtongProjectile` 도 동일.
* `MuzzleEffectScale` 기본값 2.0 은 Niagara muzzle flash 기준이었다. P_Explosion 기준으로는
  과할 수 있으니 인게임에서 확인 후 조정 필요.
