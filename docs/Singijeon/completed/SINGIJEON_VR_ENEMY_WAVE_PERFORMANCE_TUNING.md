# 작업

신기전 적군 돌진 Wave VR 최적화

# 구현 내용

- 논리 병력 수 45명과 3개 소대 구성은 유지했다.
- 실제 Skeletal Character를 10명에서 3명으로 줄이고 나머지 42명을 HISM 대리체로 처리한다.
- HISM Transform 갱신 주기를 15Hz에서 10Hz로 낮췄다.
- HISM 컬링 범위를 5000~12000cm로 제한했다.
- Wave가 생성하는 실제 적의 동적 그림자를 비활성화하고, 화면에 보일 때만 애니메이션 포즈를 갱신한다.

# 변경 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Enemy/SingijeonEnemyWaveActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonEnemyWaveTests.cpp
Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap
Scripts/ConfigureSingijeonEnemyWave.py
Scripts/VerifySingijeonEnemyWave.py
docs/Singijeon/specs/VR_INTERACTION.md
```

# 테스트 결과

- UE 5.8 Development Editor 빌드 성공
- `LV_Singijeon` Enemy Wave 설정 검증 성공
- 전체 `SuwonSiegeContestVR` 자동화 테스트 15/15 성공

# 남은 문제

- Quest 3 실기기에서 `stat unit`, `stat gpu`로 프레임타임을 측정해 실제 적 수를 0~3 사이에서 최종 조정한다.
