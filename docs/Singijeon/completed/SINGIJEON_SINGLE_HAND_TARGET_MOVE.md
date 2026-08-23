# 작업

신기전 화차 한 손 드래그와 목표 위치 가이드 구현

# 구현 내용

- 좌·우 어느 손이든 한 손만 Grab하면 화차가 손 이동량을 따라가도록 수정했다.
- 두 번째 손을 추가하거나 한 손을 놓을 때 기준점을 다시 잡아 위치가 튀지 않게 했다.
- 장전 시 화차 시작 위치 기준 250cm 앞 바닥에 발광 목표 마커를 표시한다.
- 화차가 목표의 55cm 반경에 들어오면 `Hwacha_Aim`을 성공 보고하고 마커를 숨긴다.
- 기존 손잡이 Grab 가이드는 유지하고 한 손 운반 중에는 숨도록 했다.
- 적군 쪽 원형은 42개 HISM 대리 적군에 사용한 임시 `SM_MannequinTarget` 형상임을 확인했다.

# 변경 파일

- `TwoHandCarryComponent.h/.cpp`
- `SingijeonHwachaActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- `BP_SingijeonHwacha`, `LV_Singijeon`
- `ConfigureSingijeonHandleHighlights.py`
- `VerifySingijeonSingleHandTargetMove.py`

# 주요 결정 사항

기존 Blueprint의 `BeginGrip/EndGrip` 연결과 에셋 호환성을 유지하기 위해 컴포넌트 이름은 그대로 두고 운반 조건만 단일 손까지 확장했다.

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- 좌·우 단독 Grip 이동 및 목표 도착 자동화 테스트 성공
- Blueprint/레벨 목표 마커 검증 성공
- 전체 자동화 테스트 14/14 성공

# 남은 문제

Quest 3 VR Preview에서 목표 마커의 실제 시인성과 250cm 이동 거리를 최종 확인할 수 있다.
