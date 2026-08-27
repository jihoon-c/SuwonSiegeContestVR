# 작업

적군 돌진 상태에 `Troop_march_2` 행군음을 연결하고 도망 단계에서 정지하도록 구현했다.

# 구현 내용

- Enemy Wave에 `MarchAudioComponent`, `MarchSound`, `MarchSoundVolume`을 추가했다.
- 기본 사운드는 `/GF_Singijeon/Asset/Sound/Effect/Troop_march_2`다.
- `Charging`에서 재생하며 원본 클립 종료 시 돌진 상태라면 반복한다.
- `Panicking`, `Retreating`, `Retreated`, `Ready`, `StopWave`, `EndPlay`에서 정지한다.
- Wave 3개가 동일 음원을 중복 재생하지 않도록 대표 Wave 한 개만 행군음을 담당한다.

# 변경 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 주요 결정 사항

행군음은 비공간화된 공용 전투 연출음으로 한 번만 재생한다. 에디터에서 각 Wave의 `March Sound`와 `March Sound Volume`을 교체할 수 있다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development`: 성공
- `SuwonSiegeContestVR.GF_Singijeon.EnemyWave`: 3/3 성공

# 남은 문제

없음.
