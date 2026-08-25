# 공용 남한산성 Landscape 재사용 계획

## 목적

`/Game/Maps/Main/L_NamhansanseongLandscape`를 Main 외의 임의 레벨에서도 같은 Landscape 서브레벨로 연결할 수 있게 한다.

## 현재 상태

- 지형 전용 맵은 Landscape 1개와 Streaming Proxy 121개로 구성되어 있다.
- `L_Main`은 해당 맵을 `LevelStreamingAlwaysLoaded`로 이미 참조한다.
- 체험 레벨에는 자체 Landscape가 있을 수 있으므로 자동으로 다른 레벨을 변경하면 안 된다.

## 구현 범위

- 현재 에디터에서 열린 레벨에 공용 Landscape Streaming Level을 추가하는 `Suwon.AttachNamhansanseongLandscape` 명령을 추가한다.
- 같은 Landscape가 이미 연결되어 있으면 아무 것도 추가하지 않는 멱등 동작으로 만든다.
- 임시 레벨에서 연결, 저장, 재로드, Landscape/Streaming 참조 수를 검증한다.
- 사용 방법과 결과를 Main 문서에 기록한다.

## 비범위

- 기존 체험 레벨의 자체 Landscape를 제거하거나 이동하지 않는다.
- Landscape를 독립 복제하는 기능은 포함하지 않는다.

