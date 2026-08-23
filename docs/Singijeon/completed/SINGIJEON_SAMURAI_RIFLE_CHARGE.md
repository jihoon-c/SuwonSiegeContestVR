# 신기전 적군 Samurai Rifle Charge 적용

## 완료 내용

- `Low_Poly_Samurai`를 복제한 VR용 `SKM_Low_Poly_Samurai_VR` 생성
- VR용 메시 LOD 3단계 생성, Enemy Wave의 최소 LOD를 1로 설정
- Manny/Quinn의 `MF_Rifle_Jog_Fwd`를 Samurai Skeleton으로 IK 리타겟
- 전경 적군과 GPU 인스턴스 적군 모두 동일한 Samurai 메시와 Rifle Jog 사용
- 12개 애니메이션 변형에 `0.86~1.14` 재생 속도와 서로 다른 시작 위상 적용
- 45명 중 실제 Actor는 3명만 유지하고 나머지 42명은 GPU 스켈레탈 인스턴스로 유지
- 동적 그림자 비활성화, 화면 크기 기반 애니메이션 컬링과 10Hz 위치 갱신 유지

## 주요 자산

- `SKM_Low_Poly_Samurai_VR`
- `IK_Manny_Rifle_Source`
- `IK_LowPolySamurai_Target`
- `RTG_MannyRifle_To_LowPolySamurai`
- `MF_Rifle_Jog_Fwd_Samurai`
- `DA_SingijeonSamuraiRifleRun_GPU`

경로: `/GF_Singijeon/Gameplay/Enemy/Samurai/`

## 코드와 레벨

- `ASingijeonEnemyWaveActor`: Samurai 기본 자산, LOD 하한, 속도 분산 설정 추가
- `LV_Singijeon`: `Singijeon_EnemyWave`에 위 자산과 설정 저장
- 인원 및 절차 제한: 기존 45명/전경 3명/절차별 접근 제한 유지

## 검증

- `SuwonSiegeContestVREditor Win64 Development`: 성공
- `SuwonSiegeContestVR.GF_Singijeon.EnemyWave.ScaleAndVolley`: 성공
- `Scripts/VerifySingijeonSamuraiEnemyWave.py`: Skeleton, IK 자산, LOD 3개,
  12개 변형, 속도 범위, 레벨 연결 전 항목 성공

실기기에서는 Quest 3 Preview로 손·총 자세의 시각 품질과 LOD 전환 거리를 최종 확인한다.
