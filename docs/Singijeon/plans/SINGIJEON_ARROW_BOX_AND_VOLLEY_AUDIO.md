# 목적

상태: 완료

상자에 담긴 신기전 화살을 레벨에 배치할 수 있는 경량 ISM 프리팹으로 제공하고, 화차가 화살 한 발을 발사할 때마다 교체 가능한 효과음을 재생한다.

# 현재 상태

- 화차 내부에는 최초 장전 뒤 6 x 15(90발) 자동 장전 ISM이 이미 존재한다.
- 레벨 장식/보급품 용도의 독립 화살 상자 Actor는 없다.
- 일제 발사에는 발사 사운드 설정이 없다.

# 구현 범위

- `BP_SingijeonArrowBox_Instanced` Blueprint Actor 생성
- 나무 트레이와 화살 6 x 15개를 Static Mesh + ISM으로 구성
- 화차에 발당 `Arrow Launch Sound`와 볼륨/피치 범위 변수 추가
- 실제 장전 탄약과 자동 장전 ISM 탄약 모두의 성공 발사 뒤 공통 사운드 재생

# 변경 예정 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Singijeon/SingijeonHwachaActor.*
Plugins/GameFeatures/GF_Singijeon/Private/Tests/SingijeonHwachaTests.cpp
Scripts/CreateSingijeonArrowBoxInstancedPrefab.py
Scripts/VerifySingijeonArrowBoxInstancedPrefab.py
docs/Singijeon/specs/VR_INTERACTION.md
docs/Singijeon/completed/SINGIJEON_ARROW_BOX_AND_VOLLEY_AUDIO.md
```

# 구현 단계

1. 기존 화살 메시/런타임 머터리얼로 독립 ISM 프리팹을 생성한다.
2. 화차 발사 성공 경로에 공통 사운드 헬퍼를 연결한다.
3. Editor 빌드과 프리팹 검증을 수행한다.

# 다른 Feature에 미치는 영향

- 변경은 `GF_Singijeon` 내부에 한정한다.
- 사운드 변수의 기본값은 비워 두므로 기존 레벨 동작은 바뀌지 않는다.

# 검증 방법

- UE Editor 빌드
- 자동화 테스트로 사운드 변수 기본값과 발사 흐름을 확인
- 프리팹 컴포넌트/90개 Instance/머터리얼 설정을 에디터 스크립트로 재조회

# 결과

- `/GF_Singijeon/Gameplay/Props/BP_SingijeonArrowBox_Instanced`를 생성했다.
- 트레이 5개 Static Mesh와 신기전 화살 90개(6 x 15) ISM으로 구성했으며, 표시용 화살에는 충돌과 개별 그림자를 사용하지 않는다.
- `ASingijeonHwachaActor`의 `Arrow Launch Sound`와 볼륨/피치 범위가 실제/자동 장전 화살 모두에 적용된다.
- UE 5.8 Editor 빌드, 프리팹 검증, `AutoFillGridConfiguration` 자동화 테스트를 통과했다.
