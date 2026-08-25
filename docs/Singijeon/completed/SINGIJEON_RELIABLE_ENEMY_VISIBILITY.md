# 작업

신기전 적군이 실험적 GPU 스켈레탈 인스턴스 렌더 실패로 보이지 않는 문제를 제거했다.

# 구현 내용

- 45명 중 기존 피격 Actor 3명은 유지했다.
- 나머지 42명은 일반 `USkeletalMeshComponent`로 확실히 렌더한다.
- 8개 Animation Leader만 리타겟 Rifle Jog를 평가하고 나머지 메시가 Leader Pose를 공유한다.
- 42개 프록시는 LOD1, 10Hz 이동, 비가시 Pose Tick 정지, 충돌·그림자·Decal·Navigation 비활성화를 적용한다.
- 기존 `UInstancedSkinnedMeshComponent` 경로는 `Use GPU Instanced Crowd` 선택 옵션으로 유지하되 기본값과 `LV_Singijeon` 설정을 `false`로 변경했다.
- 레벨의 적군 배치, Samurai 메시, 리타겟 애니메이션과 8개 포즈 리더 설정을 재저장했다.

# 변경 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- `ConfigureSingijeonEnemyWave.py`
- `VerifySingijeonSamuraiEnemyWave.py`
- `/Game/Maps/LV_Singijeon`

# 주요 결정 사항

Quest와 일반 에디터에서의 가시성을 우선해 표준 Skeletal Mesh 렌더 경로를 기본으로 사용한다. Actor/AI 45개를 생성하지 않고 하나의 Wave Actor가 42개 경량 컴포넌트를 관리하며 애니메이션 평가는 8개로 제한한다.

# 테스트 결과

- Editor 빌드 성공
- Samurai 메시·리타겟 애니메이션·레벨 설정 검증 성공
- Enemy Wave: 45명 논리 구성, 3명 Actor, 42명 가시 Skeletal Proxy 검증 성공
- `GF_Singijeon` 자동화 테스트 6/6 성공

# 남은 문제

- Quest 3 실기기에서 최종 프레임 타임을 확인하고 필요하면 `Shared Pose Leader Count`를 6 또는 4로 낮출 수 있다.
