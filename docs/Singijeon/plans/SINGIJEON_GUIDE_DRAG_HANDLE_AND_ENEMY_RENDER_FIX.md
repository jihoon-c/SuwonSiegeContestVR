# 목적

인터랙션 가이드 위치를 레벨 뷰포트에서 직접 이동·저장할 수 있게 하고, `LV_Singijeon`에서 적군이 실제 플레이 중 보이지 않는 원인을 제거한다.

# 현재 상태

- 완료.
- Core 결과: `docs/Core/completed/SCENARIO_GUIDE_VIEWPORT_DRAG.md`
- 신기전 결과: `docs/Singijeon/completed/SINGIJEON_ENEMY_RENDER_VISIBILITY_HARDENING.md`

# 구현 범위

- Core Scenario Interactor용 에디터 Component Visualizer와 이동 핸들
- 이동 결과를 `Guide Anchor Offset`에 저장하고 Undo/Redo 지원
- `LV_Singijeon` 적군 Actor/Root/렌더/배치 상태 진단 및 런타임 가시성 보정
- 관련 자동화 테스트와 레벨 검증

# 변경 예정 파일

- `ScenarioInteractableComponent.h/.cpp`
- `SuwonSiegeContestVREditor` 모듈 및 신규 Component Visualizer
- `SingijeonEnemyWaveActor.h/.cpp`, 테스트/검증 스크립트
- 필요 시 `/GF_Singijeon/Maps/LV_Singijeon`
- 관련 Core/Singijeon 문서

# 구현 단계 (완료)

1. 가이드 앵커 화살표 클릭용 Visualizer를 등록한다.
2. 이동 기즈모 Delta를 앵커 월드 위치와 Offset에 반영하고 트랜잭션으로 저장한다.
3. 실제 레벨 Enemy Wave의 숨김, Root visibility, 메시 머터리얼, 월드 위치를 검사한다.
4. Wave 준비 시 부모 Actor까지 명시적으로 표시하고 사람 크기·Bounds·거리 컬링을 보정한다.
5. 빌드와 Core/GF 자동화 테스트, 레벨 검증을 수행한다.

# 다른 Feature에 미치는 영향

가이드 핸들은 Core의 모든 `ScenarioInteractableComponent`에서 사용할 수 있다. 런타임 위치 계산 방식은 유지한다. 적군 수정은 `GF_Singijeon`에만 적용한다.

# 검증 방법

- Editor 빌드
- 가이드 앵커 위치 변환/저장 자동화 테스트
- GF_Singijeon Enemy Wave 렌더 상태 테스트
- `LV_Singijeon` 설정 및 Map Check
