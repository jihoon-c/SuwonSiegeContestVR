# 목적

Quest 3 VR에서 신기전 적군 돌진 Wave의 CPU·GPU 부하를 줄인다.

# 상태

완료

# 현재 상태

- 45명 중 10명이 Skeletal Character이고 35명이 HISM 대리체다.
- HISM 전체 Transform은 15Hz로 갱신된다.
- VR 프레임 저하가 적군 표시 시 발생한다.

# 구현 범위

- 실제 Character를 3명으로 제한한다.
- HISM 갱신을 10Hz로 낮추고 거리 컬링을 축소한다.
- Wave가 생성한 실제 Character의 동적 그림자를 비활성화한다.
- 레벨 배치 Actor와 자동화 테스트를 새 값으로 갱신한다.

# 검증 방법

- UE 5.8 Development Editor 빌드
- Enemy Wave 에셋 검증
- 전체 자동화 테스트
