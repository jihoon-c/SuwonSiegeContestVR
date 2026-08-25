# 작업

신기전 발사 화살에 교체 가능한 Niagara 연기 궤적 슬롯과 상대 Transform 설정을 추가했다.

# 구현 내용

- `ASingijeonProjectileActor`의 `ProjectileMesh` 자식으로 `FlightTrailEffect`를 추가했다.
- `Flight Trail System`에 임의의 Niagara System을 지정할 수 있다.
- `Flight Trail Relative Location/Rotation/Scale`로 부착 위치와 방향, 크기를 조절할 수 있다.
- 이펙트는 발사 시에만 활성화하고 장전, 언로드, 유효 피격 시 비활성화한다.
- System이 비어 있으면 기존 발사 동작만 수행한다.

# 변경 파일

- `SingijeonProjectileActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- `ConfigureSingijeonProjectileFlightTrail.py`
- `VerifySingijeonProjectileFlightTrail.py`
- `/GF_Singijeon/Gameplay/BP_SingijeonArrow`

# 주요 결정 사항

Niagara 에셋을 코드에 고정하지 않고 화살 Blueprint Class Defaults에서 교체하도록 구성했다. 따라서 연기, 불꽃, 빛 궤적을 코드 변경 없이 사용할 수 있다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development` 빌드 성공
- `BP_SingijeonArrow` 컴포넌트, 부착 관계, 기본 비활성 상태 및 편집 변수 검증 성공
- `SuwonSiegeContestVR.GF_Singijeon` 자동화 테스트 6/6 성공

# 남은 문제

- 실제 사용할 Niagara 연기 에셋 선택과 Quest 3 실기기 성능 확인은 콘텐츠 설정 단계에서 필요하다.
