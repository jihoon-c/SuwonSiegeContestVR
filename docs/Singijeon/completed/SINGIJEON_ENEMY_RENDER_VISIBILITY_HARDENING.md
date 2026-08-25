# 작업

`LV_Singijeon`에서 적군이 생성됐지만 플레이어에게 보이지 않던 문제를 렌더·크기 관점에서 보강했다.

# 구현 내용

- 실제 레벨 Wave를 임시 준비해 42개 Reliable Skeletal Proxy의 생성·위치·표시 상태를 검사했다.
- Samurai 원본 메시 높이가 약 100cm인데 기존 0.9 배율로 약 90cm만 표시되던 상태를 확인했다.
- `Desired Enemy Height` 기본 175cm를 기준으로 임포트 메시 높이를 자동 정규화한다.
- Wave Actor와 SceneRoot의 숨김 상태를 활성화 시 명시적으로 해제한다.
- Skeletal Proxy의 거리 컬링을 막고 Bounds를 보강해 VR 렌더 컬링 누락을 방지한다.
- 기존 LOD1, 8개 Pose Leader 공유, 10Hz 이동 최적화는 유지한다.

# 변경 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- `InspectSingijeonEnemyVisibility.py`

# 주요 결정 사항

LOD를 높이거나 45개 AI Actor를 생성하지 않는다. 원본 메시 임포트 크기만 사람 키에 맞추고 기존 경량 렌더 구조를 유지한다.

# 테스트 결과

- Editor 빌드 성공
- GF_Singijeon 자동화 테스트 6/6 성공
- `LV_Singijeon`: 42개 Proxy 생성, `Visible=true`, `HiddenInGame=false`
- 모든 LOD에 렌더 Section 1개 및 Opaque Samurai 머터리얼 확인
- 실제 Proxy Scale 약 1.53~1.63으로 사람 크기 정규화 확인

# 남은 문제

- Quest 3 헤드셋의 실제 시야/레벨 지형 Occlusion은 실기기에서 최종 확인한다.
