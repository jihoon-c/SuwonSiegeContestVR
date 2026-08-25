# 목적

신기전 레벨에서 준비 상태의 적군이 보이지 않는 문제, 횃불 Grab 단계 안내 누락, 점화 시 Niagara 렉을 함께 해결한다.

상태: 완료 (`completed/SINGIJEON_ENEMY_TORCH_GUIDE_FIRE_HITCH_FIX.md` 참고)

# 현재 상태

- Enemy Wave는 45명을 생성하지만 `PrepareWave()` 직후 모든 비주얼을 숨기며 첫 장전 전까지 다시 표시하지 않는다.
- `INT_04`에는 Grab 가이드 문구가 정의되어 있으나 레벨 에셋 설정을 재검증할 필요가 있다.
- `NS_Fire`가 화차 Static Mesh를 CPU emitter에서 샘플링하지만 해당 메시의 `Allow CPU Access`가 꺼져 있어 VR PIE에서 매 프레임 경고가 발생한다.
- 횃불 및 도화선 Niagara는 활성화 순간 초기화 경로가 일부 남아 있다.

# 구현 범위

- Enemy Wave 준비 상태에서도 대기 중인 적군 비주얼 표시
- `INT_04` Grab 가이드 설정 및 지속 표시 검증
- 점화 VFX 사전 준비/재사용과 Niagara CPU 접근 경고 제거

# 변경 예정 파일

- `SingijeonEnemyWaveActor.h/.cpp` 및 테스트
- `SingijeonHwachaActor.cpp` 및 점화 테스트
- `ConfigureSingijeonEnemyWave.py`, `ConfigureSingijeonCarryFuse.py`, 검증 스크립트
- 신기전 시나리오/완료 문서

# 구현 단계

1. Ready 상태 적군 표시 옵션을 추가하고 기본 활성화한다.
2. 횃불 Grab 인터랙션의 GuideAction/Text를 에셋에 다시 적용하고 검증한다.
3. 화차 메시 CPU 접근을 활성화해 `NS_Fire`의 매 프레임 실패/로그 경로를 제거한다.
4. 횃불 및 도화선 Niagara를 BeginPlay에서 준비하고 점화 시 Pause/Rendering만 전환한다.
5. C++ 자동화 테스트와 에디터 검증 스크립트를 실행한다.

# 다른 Feature에 미치는 영향

Enemy/VFX 변경은 `GF_Singijeon` 내부에 한정한다. 공통 Interaction Guide 구조와 `docs/ARCHITECTURE.md`는 변경하지 않는다.

# 검증 방법

- C++ 빌드 및 Singijeon 자동화 테스트
- `LV_Singijeon` 에셋 검증 스크립트
- VR PIE 로그에서 `NiagaraStaticMeshDataInterface ... does not allow CPU access` 반복 경고가 사라지는지 확인
