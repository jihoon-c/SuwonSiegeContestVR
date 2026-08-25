# 작업

신기전 적군 애니메이션이 런타임에서 정지하던 문제를 수정했다.

# 구현 내용

- `SKM_Low_Poly_Samurai_VR`의 LOD 0~2에 `Optimize for Instancing`을 적용하고 재빌드했다.
- 프로젝트 소유 `material_0` Material Instance에 Instanced Skinned Mesh 사용 플래그를 Override했다.
- 공용 Interchange 부모 Material은 변경하지 않도록 원래 설정으로 복구했다.
- 수정 및 회귀 검증 스크립트에 메시 빌드 설정, 머터리얼 Override, 애니메이션 Bone Track 검사를 추가했다.

# 변경 파일

- `SKM_Low_Poly_Samurai_VR.uasset`
- `material_0.uasset`
- `Scripts/FixSingijeonSamuraiAnimationAssets.py`
- `Scripts/VerifySingijeonSamuraiEnemyWave.py`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 주요 결정 사항

42명의 원거리 병력은 기존 GPU 스켈레탈 인스턴싱 구조를 유지한다. 문제 원인은 Wave 로직이나 IK 리타겟이 아니라, Transform Provider가 요구하는 메시 빌드 옵션 누락이었다.

# 테스트 결과

- Samurai 메시 LOD 0~2 `Optimize for Instancing`: 통과
- Rifle Jog 재생 길이 및 Bone Track 존재 검사: 통과
- GPU Provider 12개 변형 및 레벨 참조 검사: 통과
- 프로젝트 Material Instance 사용 플래그 Override 검사: 통과
- `SuwonSiegeContestVR.GF_Singijeon.EnemyWave.ScaleAndVolley`: 성공

# 남은 문제

Quest 3 실기기에서 최종 시각 확인이 필요하다. 화면 크기 0.006 미만의 매우 먼 적군은 성능을 위해 의도적으로 애니메이션이 정지한다.
