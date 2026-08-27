# 조선군 Kneel/Point 공용 VR 메시 완료

## 작업

`JosunGoonKneel`과 `JosunGoonPoint`를 하나의 VR 전용 스켈레탈 메시에서 재생하도록 최적화 에셋을 생성했다.

## 구현 내용

- 공용 메시 `SKM_JosunGoon_VR` 생성
- LOD 3개 생성 및 전 LOD `Optimize For Instancing` 적용
- Point 애니메이션 공용 Skeleton 복제
- Reference Pose가 다른 Kneel 애니메이션은 IK Retarget으로 변환
- 원본 메시, Skeleton, Physics Asset, 애니메이션 보존

## 변경 파일

- `/GF_Singijeon/Gameplay/Characters/JosunGoon/*`
- `CreateJosunGoonVROptimizedAssets.py`
- `VerifyJosunGoonVROptimizedAssets.py`
- `InspectJosunGoonAssets.py`

## 주요 결정 사항

두 원본 Skeleton의 Bone 이름과 계층은 같지만 Reference Pose 일부가 다르므로 강제 Skeleton 교체를 사용하지 않았다. Kneel은 별도 IK Rig/Retargeter를 통해 Point 기반 공용 Skeleton으로 변환했다.

## 테스트 결과

- 공용 메시 및 두 애니메이션 로드 성공
- LOD 3개, Material 유지, 전 LOD 인스턴싱 최적화 확인
- 두 애니메이션이 공용 메시 Skeleton을 사용하는지 확인
- Point 2.767초, Kneel 4.667초 모션 데이터 확인

## 남은 문제

없음. 실제 Actor에 사용할 때 충돌, 그림자, 거리 컬링은 배치 수와 시야 거리에 맞춰 Component에서 조절한다.
