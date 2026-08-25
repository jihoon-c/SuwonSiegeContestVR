# 목적

`Singijeon_EnemyWave`의 적군 편대를 실행 전에 레벨 에디터에서 확인하고, 횃불을 수평으로 들 때 화염이 손잡이 전체에 퍼지는 문제를 제거한다.

# 현재 상태

- 완료.
- 결과: `docs/Singijeon/completed/SINGIJEON_ENEMY_EDITOR_PREVIEW_AND_TORCH_FLAME.md`

# 구현 범위

- Enemy Wave 에디터 전용 편대 프리뷰 및 표시 옵션
- 실제 런타임 편대와 같은 위치·크기·랜덤 Seed 적용
- 횃불 전용 점 발화 효과로 교체하거나 Niagara의 메시 샘플링 대상을 점화부로 제한
- Blueprint/레벨 설정과 테스트·문서 갱신

# 변경 예정 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `IgnitionSourceActor.h/.cpp`
- 신기전 설정/검증 스크립트 및 Blueprint 에셋
- GF_Singijeon 테스트와 문서

# 구현 단계 (완료)

1. Enemy Wave가 Construction에서 편대 프리뷰를 생성·갱신하도록 한다.
2. 프리뷰는 EditorOnly/Transient/충돌 없음으로 패키징과 런타임 비용을 막는다.
3. `NS_Fire`의 사용자 파라미터와 샘플링 방식을 검사한다.
4. 횃불 끝점만 발화하도록 전용 효과 구성과 부착 Transform을 적용한다.
5. 빌드, 자동화 테스트, Blueprint/레벨 검증을 수행한다.

# 다른 Feature에 미치는 영향

두 수정 모두 `GF_Singijeon` 내부에 한정된다.

# 검증 방법

- Editor 빌드
- Enemy Wave 프리뷰 수·EditorOnly 상태 검증
- 횃불 효과가 SourceMesh가 아닌 IgnitionArea 기준으로 제한되는지 검증
- GF_Singijeon 자동화 테스트
