# Main 남한산성 Landscape 이식 완료

**완료일**: 2026-08-25

## 결과

- `/Game/Namhansanseong/Maps/Demo_Namhansanseong`의 Landscape 지형을 Main 전용 맵 `/Game/Maps/Main/L_NamhansanseongLandscape`로 분리했다.
- 지형 전용 맵에는 Landscape 1개, Landscape Streaming Proxy 121개와 World Data Layers만 남겼다.
- 원본의 `/Game/Namhansanseong/Materials/Landscape/MI_Landscape`와 Layer/Height 데이터 참조를 유지했다.
- `L_Main`에 지형 맵을 `LevelStreamingAlwaysLoaded`로 연결하고 기존 64 Component 평면 Landscape를 제거했다.
- 기존 Main 교육 Manager 1개, PlayerStart 1개, 공심돈·신기전 이동 Trigger 2개를 유지했다.
- PlayerStart와 이동 Trigger를 Demo의 기존 안전 시작 지점 `(-23078.78, 8513.44, 4858.86)` 기준으로 이동했다.

## 에디터 편집 위치

- 교육 흐름/Trigger: `/Game/Maps/Main/L_Main`
- 지형 Sculpt/Paint/Material: `/Game/Maps/Main/L_NamhansanseongLandscape`
- Landscape Material: `/Game/Namhansanseong/Materials/Landscape/MI_Landscape`

## 검증

- `L_Main` 새 프로세스 재로드 성공
- 전체 로드 액터 129개
- 지형 서브레벨 액터 123개
- Landscape 계열 액터 122개
- `MainEducationScenarioManagerActor` 1개
- `PlayerStart` 1개
- `ExperienceTravelTriggerActor` 2개
- 기존 평면 Landscape 없음
- `MI_Landscape` 참조 정상

검증 스크립트: `Scripts/VerifyMainNamhansanseongLandscape.py`
