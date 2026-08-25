# 신기전 런타임 적군 컴포넌트 등록 수정

상태: 완료 (2026-08-24)

# 목적

에디터 프리뷰에서는 보이지만 PIE/VR 런타임에서 적군이 보이지 않는 문제를 해결한다.

# 현재 상태

- `LV_Singijeon`에는 Enemy Wave와 Samurai 메시, 애니메이션, 45명 설정이 정상 배치되어 있다.
- `PrepareWave()`는 성공하며 3개 Actor와 42개 Skeletal Mesh Proxy를 만든다.
- 에디터 프리뷰는 `AddInstanceComponent`를 사용하지만 런타임 Proxy는 Actor Instance Component로 등록하지 않는다.

# 구현 범위

- 런타임 Skeletal Mesh Proxy를 Actor Instance Component로 정식 등록한다.
- 메인 렌더 패스, Bounds, Render State를 명시적으로 갱신한다.
- 제거 시 Instance Component 목록에서도 해제한다.
- 자동화 테스트에서 등록·가시성·렌더 패스 상태를 검증한다.

# 변경 파일

- `SingijeonEnemyWaveActor.cpp`
- `SingijeonEnemyWaveTests.cpp`
- 신기전 관련 계획/완료/명세 문서

# 구현 결과

1. 런타임 Proxy 생성 수명주기를 에디터 프리뷰와 동일한 Actor 등록 방식으로 변경했다.
2. 표시 활성화 시 Bounds와 Render State를 갱신하도록 했다.
3. 기존 테스트에 Actor Instance Component 소유권·등록·가시성 검증을 추가했다.
4. Editor 빌드와 `GF_Singijeon` 자동화 테스트 7개가 통과했다.

# 다른 Feature에 미치는 영향

`GF_Singijeon` Enemy Wave에만 적용하며 Core와 다른 Game Feature에는 영향을 주지 않는다.

# 검증 방법

- `SuwonSiegeContestVREditor Win64 Development` 빌드
- `SuwonSiegeContestVR.GF_Singijeon` 자동화 테스트
- `LV_Singijeon` 배치/생성 검증
