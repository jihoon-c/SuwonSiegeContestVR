# 작업

`Singijeon_EnemyWave`의 적군 메시가 보이지 않고 배치 인스턴스에서 교체하기 어려운 문제 수정

# 구현 내용

- `Proxy Skeletal Mesh`를 Details 패널에서 명확히 찾을 수 있도록 `Enemy Skeletal Mesh`로 표시한다.
- 배치된 Wave 인스턴스와 Blueprint 기본값에서 메시를 모두 교체할 수 있다.
- 메시, 애니메이션, 스케일, LOD 관련 값을 변경하면 에디터 프리뷰 병사를 즉시 다시 만든다.
- `Refresh Enemy Visuals` Call In Editor 버튼을 추가했다.
- Blueprint 런타임 교체용 `SetEnemySkeletalMesh` 함수를 추가했다.
- 이미 생성된 전경 병사와 경량 프록시 병사에도 새 메시를 재적용한다.
- 신규 Wave의 기본 메시/애니메이션을 정상화된 Samurai 에셋으로 변경했다.

# 변경 파일

- `SingijeonEnemyWaveActor.h`
- `SingijeonEnemyWaveActor.cpp`

# 주요 결정 사항

- 레벨에 배치된 Enemy Wave Actor의 Transform은 수정하지 않았다.
- 생성 병사 컴포넌트를 직접 편집하지 않고 Wave Actor의 `Enemy Skeletal Mesh` 한 곳에서 전체 외형을 관리한다.

# 테스트 결과

- UnrealHeaderTool 통과
- `SingijeonEnemyWaveActor.cpp` 컴파일 통과
- `GF_Singijeon` 모듈 라이브러리 링크 통과
- 전체 Editor Target은 작업 범위 밖의 기존 `VRPlayerPawn.cpp` 중괄호 오류로 최종 링크 전에 중단됨

# 남은 문제

- 프로젝트 전체 빌드를 완료하려면 `VRPlayerPawn.cpp` 482행 부근의 기존 구문 오류를 별도로 수정해야 한다.
