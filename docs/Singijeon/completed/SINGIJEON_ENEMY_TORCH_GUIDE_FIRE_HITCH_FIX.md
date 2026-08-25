# 작업

적군 표시, 횃불 Grab 가이드, 점화 VFX 렉 보완

# 구현 내용

- Enemy Wave는 첫 장전 전 `Ready` 상태에서도 45명 비주얼을 표시하고, 이동만 대기하도록 변경했다.
- `DA_Scenario_Singijeon.INT_04`에 `Grab / 횃불을 집으세요.` 가이드를 재적용했다. 가이드는 Grab 성공으로 단계가 완료될 때까지 유지된다.
- 횃불 및 화차 Static Mesh에 `Allow CPU Access`를 적용해 `NS_Fire` CPU emitter의 매 프레임 접근 실패와 로그 반복을 제거했다.
- 횃불 점화 준비 과정의 런타임 `SetAutoActivate` 호출을 제거했다.
- 화차 도화선 Niagara를 BeginPlay에서 미리 준비하고, 점화 시 재초기화 대신 Pause/Rendering 상태만 전환하도록 변경했다.

# 변경 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `IgnitionSourceActor.cpp`
- `SingijeonHwachaActor.cpp`
- `LV_Singijeon`, `DA_Scenario_Singijeon`, `BP_SingijeonHwacha`
- `hwacha`, `Burning_Wood_Torch` Static Mesh
- 관련 설정/검증 스크립트와 자동화 테스트

# 주요 결정 사항

적군은 시나리오 시작부터 배치된 모습을 보여주되 첫 장전 전에는 이동하지 않는다. 점화 VFX 외형은 유지하면서 접촉 프레임의 Niagara 재초기화와 CPU 접근 실패만 제거했다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development`: 성공
- 신기전 에셋 검증: 성공, 0 errors
- `SuwonSiegeContestVR.GF_Singijeon`: 6/6 성공
- `LV_Singijeon` Map Check: 0 errors, 0 warnings

# 남은 문제

Quest VR Preview에서 실제 HMD 프레임 타임과 점화 직후 로그를 한 번 더 확인해야 한다.
