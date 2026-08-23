# 작업

신기전 발사체를 Shared Health/Damage 계약에 연결했다.

# 구현 내용

- 발사된 화살이 Health Component 대상과 처음 충돌할 때 `ImpactDamage`를 표준 Damage로 전달한다.
- 한 발이 여러 번 피해를 적용하지 않도록 1회 가드를 추가했다.
- 피격 후 이동을 정지하고 2초 뒤 제거한다.
- `OnProjectileImpact` Delegate를 제공해 피격 VFX/SFX를 연결할 수 있게 했다.

# 테스트 결과

- Editor 빌드 성공
- 투사체 표준 피해 및 중복 방지 자동화 통과
- 신기전 기존 장전·점화·발사·Enemy Wave 자동화 통과

# 남은 문제

- 실제 타격 VFX/SFX 에셋 연결과 Quest 3 프레임 측정이 필요하다.
