# 작업

`RTG_MannyRifle_To_LowPolySamurai` Target 프리뷰에서 사무라이가 보이지 않는 문제 수정.

# 구현 내용

- 리타게터에 누락된 `IK_LowPolySamurai_Target` Target IK Rig 참조를 복구했다.
- Target Preview Mesh와 Target IK Rig의 Preview Mesh를 원본 `Low_Poly_Samurai`로 통일했다.
- 디스크에 `Target Mesh Scale = 0.0`으로 저장되어 Preview Mesh가 한 점으로 축소되던 직접 원인을 수정했다.
- 프리뷰 전용 값을 Source Offset `(0,0,0)`, Target Offset `(160,0,0)`, Target Scale `1.816`으로 고정했다.
- FBX의 실제 최상위 뼈 `Root`에 누적된 Scale `100`을 원본/최적화 메시 모두 `1`로 베이크했다.
- 뼈의 월드 위치와 메시 Bounds는 유지하고 Reference Skeleton 단위만 정상화했다.
- Source/Target IK Rig에 누락된 `LeftFootIK`, `RightFootIK`, `LeftHandIK`, `RightHandIK` Goal을 생성하고 체인에 연결했다.
- Source Root Motion Bone은 `root`, Target Root Motion Bone은 `Root`로 지정했다.
- 정상화된 Skeleton 기준으로 `MF_Rifle_Jog_Fwd_Samurai`를 다시 생성하고 기존 참조에 통합했다.
- Source IK Rig/Quinn Preview와 기존 Chain Mapping은 유지했다.
- 재생성 스크립트가 Target 참조 저장 실패를 즉시 검출하도록 검증을 추가했다.

# 변경 파일

- `RTG_MannyRifle_To_LowPolySamurai.uasset`
- `IK_LowPolySamurai_Target.uasset`
- `IK_Manny_Rifle_Source.uasset`
- `Low_Poly_Samurai.uasset`
- `Low_Poly_Samurai_Skeleton.uasset`
- `SKM_Low_Poly_Samurai_VR.uasset`
- `MF_Rifle_Jog_Fwd_Samurai.uasset`
- `Scripts/FixSingijeonSamuraiSkeletonAndIK.py`
- `Scripts/ReplaceSingijeonSamuraiRetargetAnimation.py`
- `Scripts/FixSingijeonSamuraiRetargeterPreview.py`
- `Scripts/InspectSingijeonSamuraiRetargeterState.py`
- `Scripts/CreateSingijeonSamuraiRifleRetarget.py`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 주요 결정 사항

- 최적화 메시의 LOD0~2에는 정상적인 정점/섹션 데이터가 있지만 UE 5.8 IK Retargeter 프리뷰에서 렌더되지 않았다.
- 저작용 IK Rig/Retargeter는 원본 `Low_Poly_Samurai`, 실제 적군 및 리타겟 출력 대상은 동일 Skeleton을 공유하는 `SKM_Low_Poly_Samurai_VR`로 분리했다.

# 테스트 결과

- 수정 전: Target Preview Mesh는 원본 Samurai였지만 Target IK Rig가 `None`인 상태 확인.
- Target IK Rig Root `Hips`, Retarget Chain 11개 확인.
- Source/Target Retarget Pose Root Offset이 모두 `(0, 0, 0)`인 상태 확인.
- 원본/최적화 메시의 LOD0 정점 수가 각각 351,404개로 비어 있지 않은 상태 확인.
- 새 Editor 프로세스 재로드 후 Target IK Rig와 원본 `Low_Poly_Samurai` Preview Mesh가 모두 저장된 상태 확인.
- 새 Editor 프로세스 재로드 후 Target Offset `(160,0,0)`, Target Scale `1.816` 저장 상태 확인.
- 정상화 전후 원본/최적화 메시 Bounds가 동일한 상태 확인.
- 새 Editor 프로세스에서 `Root`, `Hips`, 손, 발의 Global Scale이 `1`인 상태 확인.
- Source/Target 손·발 체인의 Goal이 모두 존재하며 `goal doesn't exist` 경고가 발생하지 않는 상태 확인.
- 기존 표준 경로 `MF_Rifle_Jog_Fwd_Samurai`에 새 애니메이션이 저장된 상태 확인.
- Python 검사 오류 없음.

# 남은 문제

- 기존에 리타게터 탭을 열어 둔 에디터에서는 탭을 닫고 다시 열어 Preview Scene을 갱신해야 한다.
- 원본 FBX 자체에는 Scale 100 계층이 남아 있으므로 향후 재임포트 후에는 `FixSingijeonSamuraiSkeletonAndIK.py`를 다시 실행해야 한다.
