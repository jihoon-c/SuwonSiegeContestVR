# 목적

Level 이동과 Level Travel을 넘어 유지되는 체험 진행 상태를 Core에서 구현한다.

> 상태: 완료 (2026-08-15)

# 현재 상태

- `BP_ScenarioManager`는 Level 내부 `Scenario -> Scene -> Interaction` 진행을 담당한다.
- Level 이동과 전역 진행 상태를 구현했다.
- `LV_Singijeon`과 `DA_Scenario_Singijeon`은 존재한다.
- 프로젝트 Main Level은 아직 존재하지 않는다.

# 구현 범위

- `UExperienceDefinition`: Experience ID, 체험 Level, 복귀 Level, 완료 후 자동 복귀 정책
- `UExperienceSubsystem`: 체험 시작, 현재 체험 활성화, 완료 보고, 복귀, 메모리 진행 상태
- `UScenarioExperienceBridgeComponent`: Scenario 완료를 Experience 완료로 연결
- `AScenarioManagerActor`: Experience 설정 노출과 Bridge 구성
- 신기전용 `DA_Experience_Singijeon`
- 자동화 테스트와 프로젝트 문서 갱신

# 변경 예정 파일

```text
Source/SuwonSiegeContestVR/Public/Core/Experience/*
Source/SuwonSiegeContestVR/Private/Core/Experience/*
Source/SuwonSiegeContestVR/Public/Core/Scenario/ScenarioExperienceBridgeComponent.h
Source/SuwonSiegeContestVR/Private/Core/Scenario/ScenarioExperienceBridgeComponent.cpp
Source/SuwonSiegeContestVR/*/Core/Scenario/ScenarioManagerActor.*
Source/SuwonSiegeContestVR/Private/Tests/*
Content/Core/Experience/Definitions/DA_Experience_Singijeon.uasset
Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap
docs/ARCHITECTURE.md
docs/DIRECTORY_STRUCTURE.md
docs/Main/STATUS.md
```

# 구현 단계

1. Core Data Asset과 Subsystem API 구현
2. PostLoadMap을 이용한 Traveling -> Active 상태 전환
3. Scenario Manager에 선택형 완료 Bridge 연결
4. 신기전 Data Asset 생성 및 Level Manager 설정
5. C++ 빌드, 자동화 테스트, 에셋 재조회

# 다른 Feature에 미치는 영향

- 각 Feature는 Core의 `UExperienceSubsystem`과 `UExperienceDefinition`만 참조한다.
- Core는 특정 `GF_*` 클래스나 Asset을 하드 참조하지 않는다.
- 다른 Feature는 자체 `DA_Experience_*`와 Level Manager 설정만 추가하면 된다.

# 검증 방법

- Win64 Development Editor 빌드
- Experience Definition Validation 및 메모리 진행 자동화 테스트
- 기존 Scenario 자동화 테스트
- `DA_Experience_Singijeon`과 `LV_Singijeon` Manager 연결 재조회

# 검증 결과

- Win64 Development Editor 빌드 성공
- `SuwonSiegeContestVR.Core` 자동화 테스트 5개 성공
- Experience 에셋과 Level Manager 연결 재조회 성공
- `L_Main` 부재로 `ReturnLevel`은 의도적으로 미지정
