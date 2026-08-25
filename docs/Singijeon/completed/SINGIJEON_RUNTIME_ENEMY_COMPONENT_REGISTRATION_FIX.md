# 작업

에디터 프리뷰와 달리 PIE/VR에서 적군 Proxy가 보이지 않을 수 있는 런타임 컴포넌트 등록 누락을 수정했다.

# 구현 내용

- 런타임 Skeletal Mesh Proxy를 Enemy Wave 액터의 Instance Component로 등록한다.
- 메인 렌더 패스와 가시성을 명시하고 Bounds/Render State를 갱신한다.
- 제거 시 액터의 Instance Component 목록에서도 정상 해제한다.
- 자동화 테스트에서 소유권, 등록, 가시성, 실제 렌더 가능 상태를 검증한다.

# 변경 파일

- `SingijeonEnemyWaveActor.cpp`
- `SingijeonEnemyWaveTests.cpp`

# 주요 결정 사항

42명 Proxy 방식을 유지해 액터 45개를 각각 생성하지 않고, 기존 VR 최적화 구조 안에서 렌더 수명주기만 보강했다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공
- `SuwonSiegeContestVR.GF_Singijeon` 자동화 테스트 7개 성공
- 런타임 Proxy가 액터 Instance Component에 포함되고 `ShouldRender()`가 참임을 검증

# 남은 문제

Quest 3 실기기에서 최종 프레임 성능과 화면 표시를 확인한다.
