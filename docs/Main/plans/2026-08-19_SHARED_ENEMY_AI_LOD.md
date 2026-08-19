# 목적

Android 스탠드얼론 VR에서 다수의 Shared Enemy를 저비용으로 운용할 수 있는 AI LOD와 재사용 기반을 제공한다.

# 현재 상태

Shared 전투 기반은 구현됐지만 AI, 적 이동, 풀링은 없다. 사용자는 원거리 적을 단순 이동·저빈도 갱신·비표시로 처리하고, 근거리 적만 렌더링 및 Behavior Tree/StateTree 기반 정밀 AI로 전환하도록 결정했다.

# 구현 범위

- Behavior Tree를 선택적으로 구동하는 Shared Enemy AI Controller
- 거리 기반 AI LOD Component: 원거리 시 시각 비활성화, 저빈도 단순 이동, 근거리 시 고빈도/BT 전환
- NavMesh/BT 없이 목표를 향하는 저비용 단순 이동 Component
- Feature가 Advance/Retreat/Construct/Assault/Archer 상태를 소유할 수 있는 상태 이벤트 Component
- 생성·파괴 급증을 피하기 위한 Generic Actor Pool 및 Poolable Interface

# 변경 예정 파일

- `Source/SuwonSiegeContestVR/Public/Gameplay/AI/**`
- `Source/SuwonSiegeContestVR/Private/Gameplay/AI/**`
- `Source/SuwonSiegeContestVR/Public/Gameplay/Pooling/**`
- `Source/SuwonSiegeContestVR/Private/Gameplay/Pooling/**`
- `Source/SuwonSiegeContestVR/SuwonSiegeContestVR.Build.cs`
- `docs/ARCHITECTURE.md`, 각 Feature 상태 문서

# 구현 단계

1. AI LOD와 단순 목표 이동의 공통 계약을 구현한다.
2. Behavior Tree Controller와 적 상태 이벤트를 추가한다.
3. Actor Pool 계약을 추가한다.
4. Editor 빌드로 UHT/C++를 검증한다.

# 다른 Feature에 미치는 영향

Shared 구현은 Feature를 참조하지 않는다. Feature는 Level/Manager에서 목표점과 상태를 설정하고, 가까운 적의 BT 또는 StateTree 확장 로직을 선택적으로 연결한다.

# 검증 방법

- Unreal Editor 타깃 빌드
- AI LOD가 Component/Controller의 선택적 Asset 참조만 사용하고, Shared → Feature 역의존이 없는지 확인
