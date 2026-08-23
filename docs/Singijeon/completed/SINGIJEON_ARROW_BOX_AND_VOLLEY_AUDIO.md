# 작업

신기전 화살 상자 ISM 프리팹 및 발사 효과음 설정

# 구현 내용

- `/GF_Singijeon/Gameplay/Props/BP_SingijeonArrowBox_Instanced`를 생성했다.
- 나무 트레이 5개와 신기전 화살 90개(6 x 15)를 `ArrowInstances` Instanced Static Mesh로 구성했다.
- 표시용 화살 ISM의 충돌과 개별 그림자를 비활성화해 VR 레벨 배치 비용을 낮췄다.
- `ASingijeonHwachaActor`에 `Arrow Launch Sound`, 볼륨, 최소/최대 피치 변수를 추가했다.
- 실제 슬롯 화살과 자동 장전 Instance 화살이 성공 발사된 직후 각각 동일한 발사 효과음을 재생하도록 연결했다.

# 변경 파일

- `SingijeonHwachaActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- `Scripts/CreateSingijeonArrowBoxInstancedPrefab.py`
- `Scripts/VerifySingijeonArrowBoxInstancedPrefab.py`
- `BP_SingijeonArrowBox_Instanced.uasset`
- `M_SingijeonArrowBox_Wood.uasset`
- `VR_INTERACTION.md`

# 주요 결정 사항

- 상자는 장식/보급품 전용이며 화차의 런타임 탄약 배열과 연결하지 않는다.
- 사운드 에셋은 화차 Blueprint 기본값 또는 레벨 배치 Actor 인스턴스에서 교체할 수 있도록 `USoundBase` 변수로 노출했다.
- 기본 사운드는 비워 두어 기존 레벨의 음향을 임의로 변경하지 않는다.

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- C++ 기반 프리팹 Actor 컴파일 성공: 트레이, 런타임 머터리얼, 90개 ISM, 충돌/그림자 비활성 구성을 생성자/Construction으로 고정
- `SuwonSiegeContestVR.GF_Singijeon.Hwacha.AutoFillGridConfiguration` 자동화 테스트 성공

# 남은 문제

- 사용할 발사 음원은 `Arrow Launch Sound`에 지정하면 된다. Quest 3 실기기에서 볼륨과 피치 범위를 최종 조정한다.
