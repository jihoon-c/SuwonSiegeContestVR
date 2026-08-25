# 공용 남한산성 Landscape 재사용 완료

## 작업

남한산성 지형 전용 맵을 Main 외의 다른 레벨에서도 중복 복사 없이 재사용할 수 있도록 에디터 연결 명령을 추가했다.

## 구현 내용

- 공용 지형: `/Game/Maps/Main/L_NamhansanseongLandscape`
- 에디터 콘솔 명령: `Suwon.AttachNamhansanseongLandscape`
- 현재 열린 레벨에 공용 지형을 `LevelStreamingAlwaysLoaded`로 추가한다.
- 같은 지형이 이미 연결된 레벨에서는 중복 Streaming Level을 만들지 않는다.
- 지형 맵 자신에게 연결하려는 요청은 차단한다.

## 사용 방법

1. 지형을 쓸 대상 레벨을 연다.
2. Output Log 또는 콘솔에서 `Suwon.AttachNamhansanseongLandscape`를 실행한다.
3. 대상 레벨을 저장한다.

또는 `Scripts/RunAttachNamhansanseongLandscape.py`를 실행한다.

## 변경 파일

- `Source/SuwonSiegeContestVREditor/Private/SuwonSiegeContestVREditor.cpp`
- `Scripts/RunAttachNamhansanseongLandscape.py`
- `Scripts/VerifySharedNamhansanseongLandscapeReuse.py`

## 검증

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공
- 이미 공용 지형이 연결된 `L_Main`에서 명령 재실행 후 지형 서브레벨 Actor 수가 123개로 유지됨

