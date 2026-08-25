# 목적

신기전 적군의 GPU 인스턴스 애니메이션이 런타임에서 정지 포즈로 표시되는 문제를 수정한다.

상태: 완료

# 현재 상태

- `DA_SingijeonSamuraiRifleRun_GPU`는 존재하지만 대상 메시가 `Optimize for Instancing` 없이 빌드되어 Provider가 런타임 등록을 거부한다.
- Samurai 머터리얼에 Instanced Skinned Mesh 사용 플래그가 없어 런타임에서 기본 머터리얼로 대체된다.

# 구현 범위

- Samurai VR 메시의 모든 LOD에 `Optimize for Instancing` 적용 및 재빌드
- 사용 머터리얼에 Instanced Skinned Mesh 사용 플래그 적용
- 검증 스크립트에 필수 빌드 설정, 머터리얼 사용 플래그, 애니메이션 데이터 검증 추가

# 변경 예정 파일

- `Scripts/FixSingijeonSamuraiAnimationAssets.py`
- `Scripts/VerifySingijeonSamuraiEnemyWave.py`
- Samurai Skeletal Mesh 및 Material 에셋

# 구현 단계

1. 문제 에셋 설정 수정
2. Provider/애니메이션 데이터 검증
3. 런타임 로그에서 Provider 거부 경고 재발 여부 확인

# 다른 Feature에 미치는 영향

`GF_Singijeon` 내부 에셋만 수정하며 Shared/Core에는 영향을 주지 않는다.

# 검증 방법

- 에셋 검증 스크립트 통과
- `LV_Singijeon` PIE 실행 로그에 `requires 'Optimize for Instancing'` 경고가 없는지 확인
