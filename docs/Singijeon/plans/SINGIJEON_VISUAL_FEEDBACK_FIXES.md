# 목적

신기전 체험에서 화로 불꽃 높이, 자동 장전 신기전 인스턴스의 머티리얼, 양손 화차 운반 중 손잡이 하이라이트를 보완한다.

# 현재 상태

- `BP_SingijeonFirePit`의 Niagara 불꽃이 화로 상단이 아닌 바닥 가까이에 표시된다.
- 화차 자동 장전용 `InstancedStaticMeshComponent`가 원본 신기전 메시의 머티리얼을 복사하지 않는다.
- `UTwoHandCarryComponent`는 양손 운반 상태를 전달하지만 화차 손잡이 시각 피드백은 없다.

# 구현 범위

- 화로 불꽃 및 점화 영역의 상대 높이 조정
- 자동 장전 인스턴스에 명시적 머티리얼 적용 및 원본 메시 머티리얼 복사 fallback 추가
- 양손 운반 상태 동안에만 표시되는 좌우 손잡이 하이라이트 컴포넌트 추가
- 관련 Blueprint 기본값과 레벨 배치 액터 갱신

# 변경 예정 파일

- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Public/Singijeon/SingijeonHwachaActor.h`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Singijeon/SingijeonHwachaActor.cpp`
- `Scripts/ConfigureSingijeonFirePitFlow.py`
- 신기전 시각 피드백 설정/검증 스크립트
- 관련 신기전 Blueprint 에셋 및 문서

# 구현 단계

1. 화로/화차 메시 범위와 현재 Blueprint 컴포넌트 설정 확인
2. 런타임 머티리얼 복사 및 손잡이 하이라이트 상태 처리 구현
3. Blueprint 기본값, 컴포넌트 위치 및 레벨 액터 반영
4. C++ 빌드, 자동화 테스트, 에셋 검증 수행
5. 완료 문서와 상태 문서 갱신

# 다른 Feature에 미치는 영향

`GF_Singijeon` 내부 클래스와 에셋만 수정한다. Core 및 다른 Game Feature 의존성은 변경하지 않는다.

# 검증 방법

- 프로젝트 Editor 타깃 빌드 성공
- 신기전 자동화 테스트 통과
- FireEffect 높이와 인스턴스 머티리얼이 Blueprint/레벨에 저장됐는지 명령형 검증
- 장전 후 안내 표시 → 양손 Grab 시 숨김 → 미완료 Drop 시 복원 → 이동 완료 시 해제 순서 확인

# 완료 상태

`Status: Complete` (2026-08-19)

- 화로 메시 상단 높이 기준으로 불꽃과 점화 영역을 `Z=102`에 배치했다.
- 자동 장전 ISM에 `M_Arrow01b`를 지정하고 런타임 원본 머티리얼 복사 fallback을 추가했다.
- 조준 인터랙션이 필요한 동안 좌우 손잡이를 안내하고, 양손 Grab 중에는 숨기며, 미완료 Drop 시 복원하도록 수정했다.
- 화차가 30cm 이동하거나 10도 회전하여 `Hwacha_Aim`이 완료되면 안내를 최종 해제한다.
- Editor 빌드, 에셋 검증, 신기전 자동화 테스트 2개가 모두 성공했다.
