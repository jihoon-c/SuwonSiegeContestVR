# 작업

신기전 화차 6 x 15 화살 자동 장전 구현

# 구현 내용

- `ASingijeonHwachaActor`에 `AutoLoadedArrowInstances`를 추가했다.
- 플레이어가 첫 화살 한 발을 장전하면 같은 메시의 Instance 89개가 자동 생성되어 총 90발로 계산된다.
- 원본 화살을 빼거나 화차를 리셋하면 자동 생성 Instance도 제거된다.
- 발사 시 Instance를 첫 화살과 같은 Actor 클래스로 순차 생성해 기존 Projectile 발사 로직을 사용한다.
- `BP_SingijeonHwacha`와 `BP_SingijeonArrow`에 이동된 실제 화차/화살 메시를 다시 연결했다.
- 기존 DA 기반 나레이션/인터랙션 흐름에 맞게 플레이 검증 스크립트를 갱신했다.

# 변경 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Singijeon/SingijeonHwachaActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonHwachaTests.cpp
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonHwacha.uasset
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonArrow.uasset
Scripts/ConfigureSingijeonAutoFillArrowGrid.py
Scripts/VerifySingijeonAutoFillArrowGrid.py
Scripts/VerifyPlayableSingijeonExperience.py
docs/Singijeon/specs/VR_INTERACTION.md
```

# 주요 결정 사항

- 실제 상호작용 대상은 첫 화살 한 발만 유지하고 나머지 89발은 ISM으로 표시해 Actor 90개 상시 배치 비용을 피한다.
- 발사 순간에만 Instance를 Projectile Actor로 변환한다.
- 6행, 15열, 행/열 간격, Offset, Mesh는 Blueprint에서 조절할 수 있다.

# 테스트 결과

- Win64 Development Editor 빌드 성공
- 한 발 장전 시 총 90발/Instance 89개, 장전 해제 시 0개 자동화 테스트 성공
- `SuwonSiegeContestVR` 자동화 테스트 6개 전부 성공
- BP/레벨 자동 장전 설정 및 실제 메시 재조회 성공
- 현재 DA 기반 신기전 시나리오와 플레이 배치 검증 성공

# 남은 문제

- 실제 화차 발사관과 정확히 맞는 간격과 Offset은 Quest PIE에서 시각 확인 후 조절할 수 있다.
