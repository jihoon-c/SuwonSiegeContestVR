# 목적

신기전으로 돌진하는 적군을 Low Poly Samurai 메시와 총을 든 달리기 애니메이션으로 교체하고, VR 성능을 유지한다.

상태: 완료 (2026-08-23)

# 구현 범위

- Samurai 스켈레톤과 Rifle Jog 애니메이션의 IK 리타겟
- 전경 적군과 GPU 프록시 메시/애니메이션 교체
- 인스턴스별 재생 속도 분산
- Skeletal LOD와 거리 컬링 하향 설정

# 검증

- 리타겟 결과의 스켈레톤·루트 이동·총 손 위치 확인
- Wave 45명에서 프록시/전경 적군 재생 확인
- Quest용 LOD, 그림자, 애니메이션 컬링 확인

# 완료 결과

- Manny Rifle Jog를 Samurai Skeleton으로 IK 리타겟
- 45명 중 전경 Actor 3명, GPU 스켈레탈 인스턴스 42명 유지
- 속도 `0.86~1.14`, 시작 위상 12종으로 개체별 반복감 감소
- 전용 Samurai 메시 3 LOD 생성, Wave 최소 LOD 1 적용
- Editor 빌드 성공
- `SuwonSiegeContestVR.GF_Singijeon.EnemyWave.ScaleAndVolley` 성공
- 자산·Skeleton·LOD·레벨 연결 검증 스크립트 전 항목 성공
