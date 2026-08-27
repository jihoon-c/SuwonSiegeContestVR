# 신기전 동시 발사 최적화 화차 편대 완료

## 작업

플레이 가능한 신기전 화차 발사와 동시에 작동하는 시각 연출용 지원 화차 편대를 구현하고 `LV_Singijeon`에 배치했다.

## 구현 내용

- 독립 배치 가능한 지원 화차 Actor 4개와 장전 신기전 264발(Actor당 1대, 6 x 11)
- Playable 화차의 `Fired` 상태를 구독해 10초 동시 발사
- 화차/장전 화살/비행 화살을 ISM 3개로 묶어 렌더링
- 화살별 Actor, 충돌, Overlap, 물리, Niagara 제거
- 비행 인스턴스 30Hz 묶음 갱신 및 수명 종료 제거
- 기존 레벨 액터 Transform을 저장 전후 비교해 변경 방지
- `Arrow Rack Transform`의 뷰포트 편집 Widget과 청록색 방향 가이드 추가
- 기본 화살 랙 위치를 로컬 Y +10cm, Z +10cm만큼 우측·상단 보정
- Playable과 지원 화차 모두 화살촉 기준 좌우 14도/상하 6도 부채꼴 발사 적용
- 발사체 메시 화살촉을 실제 분산 속도 방향으로 정렬

## 변경 파일

- `SingijeonHwachaBatteryActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- `ConfigureSingijeonOptimizedSupportBattery.py`
- `VerifySingijeonOptimizedSupportBattery.py`
- `GF_Singijeon/Content/Maps/LV_Singijeon.umap`

## 주요 결정 사항

지원 편대는 시나리오 판정과 그랩을 갖지 않는 시각 연출 전용이다. 성공 판정은 기존 Playable 화차만 담당한다. 각 지원 Actor의 Transform은 독립적으로 수정한다.

## 테스트 결과

- Editor Development C++ 빌드 성공
- 레벨 MapCheck 0 Error / 0 Warning
- 개별 Actor 4개, Actor당 1대/66발, Source 참조, 독립 Transform 검증 성공
- 우측·상단 랙 보정 및 에디터 방향 가이드 검증 성공
- `SuwonSiegeContestVR.GF_Singijeon.Hwacha.OptimizedSupportBattery` 성공
- GF_Singijeon 모듈 Compile/Link 및 런타임 로드 성공
- Editor Development 전체 프로젝트 빌드 성공
- GF_Singijeon 자동화 테스트 8개 전체 성공

## 남은 문제

없음. 실제 Quest 3 화질에 따라 배치 Actor의 Cull Distance만 현장에서 조절한다.
