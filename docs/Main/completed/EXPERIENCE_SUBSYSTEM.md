# 작업

Core Experience Level 이동 및 세션 진행도 구현

# 구현 내용

- `UExperienceDefinition` Data Asset으로 체험 ID, 체험 Level, 복귀 Level, 자동 복귀 정책을 정의한다.
- `UExperienceSubsystem`이 Soft World 기반 `OpenLevel`, 상태 전이, 완료 목록을 GameInstance 수명 동안 관리한다.
- `UScenarioExperienceBridgeComponent`가 Scenario 완료를 Experience 완료와 선택형 복귀로 연결한다.
- `DA_Experience_Singijeon`을 생성하고 `LV_Singijeon`의 Scenario Manager에 연결했다.
- 에디터에서 `LV_Singijeon`을 직접 실행해도 Experience가 활성화된다.

# 변경 파일

```text
Source/SuwonSiegeContestVR/*/Core/Experience/*
Source/SuwonSiegeContestVR/*/Core/Scenario/ScenarioExperienceBridgeComponent.*
Source/SuwonSiegeContestVR/*/Core/Scenario/ScenarioManagerActor.*
Content/Core/Experience/Definitions/DA_Experience_Singijeon.uasset
Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap
Scripts/CreateExperienceCoreAssets.py
Scripts/VerifyExperienceCore.py
```

# 주요 결정 사항

- Level 전환은 `OpenLevelBySoftObjectPtr`를 사용한다.
- 진행도는 우선 10분 단발 세션을 위한 메모리 방식으로 유지한다.
- Scenario는 Level 내부 흐름만 소유하고, Level 이동은 ExperienceSubsystem이 소유한다.
- Core는 `GF_Singijeon` 클래스를 참조하지 않으며 Data Asset과 이벤트로 연결한다.

# 테스트 결과

- Win64 Development Editor 빌드 성공
- `SuwonSiegeContestVR.Core` 자동화 테스트 5개 성공
- `DA_Experience_Singijeon` 및 `LV_Singijeon` Manager 연결 재조회 성공
- `git diff --check` 성공

# 남은 문제

- `L_Main`이 아직 없어 `DA_Experience_Singijeon.ReturnLevel`은 비어 있다.
- 앱 재실행 후 진행도 유지가 필요하면 `SaveGame` 구현이 필요하다.
- VR 전환 페이드와 로딩 화면은 별도 구현이 필요하다.
- 신기전 Scenario의 `INT_03 Hwacha_Aim`, `INT_05 Torch_Ignite` 완료 보고는 Feature 작업으로 남아 있다.
