# 목적

상태: 완료

플레이어가 신기전 화살 한 발을 화차에 장전하면 나머지 탄약이 `6 x 15`, 총 90발 구성으로 자동 장전되게 한다.

# 현재 상태

- 화차에는 실제 Actor를 받는 `DefaultAmmoSlot` 하나가 있다.
- 장전 수와 발사는 실제 Slot Actor만 대상으로 한다.
- 한 발 장전 후 나머지 탄약을 자동으로 표현하거나 발사하는 기능은 없다.

# 구현 범위

- 화차에 Arrow Instanced Static Mesh 컴포넌트 추가
- 최초 1발 장전 시 같은 메시로 나머지 89개 Instance 자동 생성
- 자동 장전 탄약을 총 장전 수와 발사 가능 상태에 포함
- 발사 시 Instance를 같은 Arrow Actor 클래스로 순차 변환하여 발사
- 장전 취소/리셋 시 자동 생성 Instance 제거
- BP 화차에 6행 x 15열과 실제 Arrow Mesh 설정

# 변경 예정 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Singijeon/SingijeonHwachaActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonHwachaTests.cpp
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonHwacha.uasset
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonArrow.uasset
Scripts/ConfigureSingijeonAutoFillArrowGrid.py
Scripts/VerifySingijeonAutoFillArrowGrid.py
docs/Singijeon/specs/VR_INTERACTION.md
docs/Singijeon/completed/SINGIJEON_AUTO_FILL_ARROW_GRID.md
```

# 구현 단계

1. ISM Grid와 자동 장전 상태 추가
2. Slot 장전/해제와 자동 채움 연결
3. Instance 순차 발사 구현
4. Blueprint와 실제 Arrow Mesh 설정
5. 빌드, 자동화 테스트, 에셋 재조회

# 다른 Feature에 미치는 영향

- 변경은 `GF_Singijeon` 내부에 한정한다.
- Core Scenario에는 기존 `Hwacha_Load`, `Hwacha_Fire` 성공 보고만 사용한다.

# 검증 방법

- Editor 빌드
- 화차 기본 Grid 용량 90 및 ISM 컴포넌트 자동화 테스트
- BP 설정과 Level 배치 Actor 재조회
- Core 및 Singijeon 자동화 회귀 테스트

# 결과

- 첫 화살 1발과 ISM 89개를 합쳐 6 x 15, 총 90발을 구성한다.
- 장전 취소, 리셋, 순차 발사 흐름을 연결했다.
- 실제 화차/화살 메시를 BP와 레벨 배치 액터에 적용했다.
- Editor 빌드, 자동화 테스트 6개, 에셋 및 시나리오 검증을 통과했다.
