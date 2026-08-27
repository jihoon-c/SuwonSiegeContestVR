# 조선군 Kneel/Point 공용 VR 메시

> 상태: 완료 (2026-08-26)

## 목적

- 새로 임포트된 `JosunGoonKneel`, `JosunGoonPoint`를 공용 스켈레탈 메시 1개로 사용한다.
- 두 애니메이션을 동일 Skeleton에서 재생할 수 있게 한다.
- 원본 LOD0 외형은 유지하고 거리별 LOD와 인스턴싱 친화 설정으로 VR 비용을 낮춘다.

## 확인한 원본 상태

- Kneel과 Point가 각각 별도 Skeletal Mesh, Skeleton, Physics Asset, Animation으로 임포트되어 있다.
- Bone 이름/계층은 같지만 Reference Pose 일부가 달라 단순 Skeleton 교체 대신 IK Retarget을 사용했다.

## 구현 범위

- 원본 에셋은 보존한다.
- `/GF_Singijeon/Gameplay/Characters/JosunGoon/`에 공용 VR 메시와 공용 애니메이션을 생성한다.
- 공용 메시에는 3개 LOD와 `Optimize For Instancing`을 적용한다.
- 물리/충돌은 사용하는 Actor Component에서 끌 수 있도록 메시 자체 Physics Asset은 제거하지 않는다.

## 변경 예정 파일

- `Scripts/CreateJosunGoonVROptimizedAssets.py`
- `Scripts/VerifyJosunGoonVROptimizedAssets.py`
- `GF_Singijeon/Gameplay/Characters/JosunGoon/*`

## 구현 단계

1. 두 Skeleton의 Bone 구조 및 애니메이션 Skeleton 검사
2. 공용 대상 Skeleton/메시 결정
3. VR 메시 복제, LOD 3개 생성, LOD 빌드 설정 최적화
4. Point는 공용 Skeleton 그대로 복제하고 Reference Pose가 다른 Kneel은 IK Retargeter로 변환
5. 로드, LOD, Skeleton 참조, 애니메이션 길이 검증

## 다른 Feature에 미치는 영향

- `GF_Singijeon` 내부에만 새 에셋을 추가한다.
- 기존 조선군 원본 및 레벨 배치는 변경하지 않는다.

## 검증 방법

- 공용 메시 LOD 수 및 Material 확인
- 두 애니메이션의 Skeleton이 공용 메시 Skeleton과 같은지 확인
- 에셋 로드/저장 오류와 참조 누락 검사
