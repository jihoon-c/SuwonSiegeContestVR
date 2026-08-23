# 목적

현재 활성 Scenario 입력 가이드를 HMD 시점 방향으로 안정적으로 표시하고, `SM_Stone_Barrier`의 고비용 Normal/Roughness 머터리얼을 사용하지 않는 경량 벽 프리팹을 제공한다.

# 상태

완료. 구현 및 검증 결과는 `docs/Core/completed/SCENARIO_INTERACTION_GUIDE_AND_VR_SUBTITLE_VISIBILITY.md`와 `docs/Singijeon/completed/OPTIMIZED_STONE_BARRIER_PREFAB.md`에 기록했다.

# 현재 상태

- 공통 `UScenarioInteractionGuideComponent`는 활성 가이드 하나만 Tick하지만 Pitch까지 회전한다.
- `SM_Stone_Barrier`는 Base Color, Normal, Roughness 텍스처 6개와 2개 머터리얼 슬롯을 사용한다.

# 구현 범위

- 활성 가이드만 30Hz로 HMD를 향하도록 Yaw 회전을 갱신한다.
- `SM_Stone_Barrier`의 Base Color만 사용하는 Fully Rough 경량 머터리얼과 1K 텍스처를 만든다.
- 소스 메시를 사용하는 `BP_Stone_Barrier_Optimized` 프리팹을 만든다.

# 변경 예정 파일

- `ScenarioInteractionGuideComponent.h/.cpp`
- `ScenarioFrameworkTests.cpp`
- 벽 경량화 구성/검증 Python 스크립트
- `GF_Singijeon/Content/Asset/NamhansanseongWall/Optimized/*`

# 검증 방법

- Core Scenario 가이드 자동화 테스트
- 경량 Blueprint, 저해상도 텍스처, 단순 머터리얼 및 메시 할당 검증 Commandlet
