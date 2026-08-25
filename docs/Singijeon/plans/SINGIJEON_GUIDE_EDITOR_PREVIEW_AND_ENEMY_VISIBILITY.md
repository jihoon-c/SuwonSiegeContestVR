# 목적

인터랙션 가이드의 실제 표시 위치를 실행 전 에디터에서 확인·조절하고, 신기전 적군이 GPU 인스턴스 렌더 경로 실패 여부와 관계없이 확실히 보이게 한다.

# 현재 상태

- 완료.
- Core 결과: `docs/Core/completed/SCENARIO_GUIDE_EDITOR_ANCHOR.md`
- 신기전 결과: `docs/Singijeon/completed/SINGIJEON_RELIABLE_ENEMY_VISIBILITY.md`

# 구현 범위

- 각 `ScenarioInteractableComponent`에 대상별 Guide Anchor Offset과 에디터 전용 시각 마커 추가
- 런타임 가이드가 선택된 Interactor의 Anchor Offset을 사용
- 적군 후방 병력을 포즈 공유 Skeletal Mesh Component로 표시하는 안정 경로 추가
- 기존 GPU 인스턴스 경로는 선택 옵션으로 유지
- 레벨/Blueprint 설정 반영과 검증

# 변경 파일

- Core `ScenarioInteractableComponent`, `ScenarioInteractionGuideComponent`, Scenario 테스트
- `SingijeonEnemyWaveActor`, Enemy Wave 테스트
- 설정/검증 스크립트와 관련 문서

# 구현 단계 (완료)

1. Interactor Bounds 상단에 에디터 전용 Anchor 마커를 생성한다.
2. 대상별 Offset을 런타임 Widget 위치 계산에 적용한다.
3. 42명 시각 프록시를 소수 Animation Leader와 다수 Pose Follower로 구성한다.
4. 충돌·그림자·AI를 끄고 LOD를 유지한다.
5. 빌드, 레벨 설정, 자동화 테스트를 수행한다.

# 다른 Feature에 미치는 영향

Guide Anchor는 Core의 모든 Scenario Interactor에서 사용할 수 있다. 기본 Offset은 기존 35cm 동작과 같아 기존 Feature 표시 위치를 유지한다. 적군 렌더 변경은 `GF_Singijeon`에만 적용한다.

# 검증 방법

- Editor 빌드
- Core Scenario 가이드 테스트
- 신기전 Enemy Wave 생성 수·가시성·포즈 공유 테스트
- `LV_Singijeon` 에셋/Map Check 및 런타임 준비 검증
