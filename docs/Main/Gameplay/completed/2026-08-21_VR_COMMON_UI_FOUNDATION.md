# 작업

VR 공용 UI 기반 및 웅성 총통 HUD 연결

## 구현 내용

- Feature-neutral `FVRHUDState`와 알림 유형 정의
- `UVRHUDComponent`에 Objective, Progress, Prompt, Notification API 및 상태 변경 이벤트 구현
- `UVRHUDWidget` Native fallback 레이아웃과 Blueprint 확장 이벤트 구현
- `/Game/Gameplay/UI/Common/WBP_VRHUD` Widget Blueprint 생성 및 컴파일
- `AVRPlayerPawn`에 기존 자막/나레이션 슬롯과 독립된 카메라 부착 `StatusHUD` 추가
- `BP_VRPlayerPawn.StatusHUD.WidgetClass`에 `WBP_VRHUD` 지정
- 웅성 장전 단계, 쑤시개 횟수, 적 Wave 진행, 성문 피격/파괴, 방어 성공 표시 연결
- HUD 상태 정규화와 Clear 계약 Automation Test 추가

## 변경 파일

- `Source/SuwonSiegeContestVR/Public/Gameplay/UI/`
- `Source/SuwonSiegeContestVR/Private/Gameplay/UI/`
- `Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`
- `Source/SuwonSiegeContestVR/Private/Tests/ScenarioFrameworkTests.cpp`
- `Content/Gameplay/UI/Common/WBP_VRHUD.uasset`
- `Content/Core/VR/Pawn/BP_VRPlayerPawn.uasset`
- `Plugins/GameFeatures/GF_OngseongCrossbow/Source/GF_OngseongCrossbow/`

## 주요 결정 사항

- 공용 HUD는 특정 Feature 클래스를 참조하지 않고 일반 텍스트와 정수 진행값만 소유한다.
- Feature 이벤트를 HUD 데이터로 바꾸는 책임은 Feature-local Adapter가 가진다.
- 지속 HUD, 자막, 나레이션 Modal은 서로 다른 World Widget 슬롯으로 유지한다.
- 기본 C++ Widget은 안전망이며 최종 시각 디자인은 Blueprint subclass로 교체할 수 있다.

## 테스트 결과

- UHT: 성공, 새 reflected type 16개 생성
- `SuwonSiegeContestVR Win64 Development`: 성공, 11 actions 전체 컴파일/링크 성공
- `SuwonSiegeContestVREditor Win64 Development`: 성공, 12 actions 전체 컴파일/링크 성공
- `SuwonSiegeContestVR.Core.VR.HUDStateContract`: 성공, Error/Warning 0
- `LV_Ongseong` PIE: `BP_VRPlayerPawn_C_0.StatusHUD.WidgetClass=WBP_VRHUD_C` 확인
- `LV_Ongseong` PIE: `StatusHUD.bVisible=true`, `bHiddenInGame=false`, DrawSize 720x360 확인
- PIE 종료 후 Map/Asset 저장 상태 유지

## 남은 문제

- VR Preview/실기기에서 HUD 거리, 크기, 시야 중첩, 한글 폰트 확인
- 공심돈 보고용 상호작용 Modal과 신기전 장전/점화 Adapter 추가
- Timer, 체력, World Anchor/손목 표시 변형은 필요 시 확장
