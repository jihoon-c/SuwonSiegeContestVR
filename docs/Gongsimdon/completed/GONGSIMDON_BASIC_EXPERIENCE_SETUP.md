# 작업

공심돈 레벨을 Core Scenario/Experience 흐름에 연결하고 Main 왕복이 가능한 기초 Scenario 01을 구성했다.

# 구현 내용

- `/GF_Gongsimdon/Maps/LV_Gongsimdon`에 Scenario Manager와 PlayerStart 설정
- `/GF_Gongsimdon/Data/DA_Scenario_Gongsimdon` 생성
- `/Game/Core/Experience/Definitions/DA_Experience_Gongsimdon` 생성
- Scenario 완료 시 `/Game/Maps/Main/L_Main` 자동 복귀
- Main 복귀 후 `MAIN_AFTER_GONGSIMDON`부터 진행 복원

# 변경 파일

```text
Plugins/GameFeatures/GF_Gongsimdon/Content/Maps/LV_Gongsimdon.umap
Plugins/GameFeatures/GF_Gongsimdon/Content/Data/DA_Scenario_Gongsimdon.uasset
Content/Core/Experience/Definitions/DA_Experience_Gongsimdon.uasset
docs/Gongsimdon/STATUS.md
```

# 주요 결정 사항

공심돈 고유 게임플레이는 Feature에 두고, Level 전환과 진행 복원은 Core Experience 시스템을 사용한다. 초기 5초 Wait 자리표시자는 야간 경계 Action Interaction 흐름으로 교체됐다.

# 테스트 결과

- 공심돈 Level/Manager/PlayerStart/Scenario/Experience 참조 검증 성공
- Main 복귀 Level과 자동 복귀 설정 검증 성공
- 전체 프로젝트 자동화 테스트 7/7 성공

# 남은 문제

- 탐지/관측 게임플레이와 전용 나레이션 구현
- VR 실기에서 PlayerStart와 공간 안전성 조정
