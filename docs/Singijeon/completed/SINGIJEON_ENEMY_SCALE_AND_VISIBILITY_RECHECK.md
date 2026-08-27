# 작업

`Singijeon_EnemyWave` 생성 병사 1.5배 확대 및 런타임 가시성 재점검

# 구현 내용

- 배치 Wave의 `Proxy Scale`을 기존 0.9의 1.5배인 `(1.35, 1.35, 1.35)`로 변경했다.
- `Desired Enemy Height=175cm`, `Show Enemies While Ready=true`, Quest-safe 독립 Enemy Actor 경로를 유지했다.
- 설정/검증/가시성 계측 스크립트에 현재 병사 배율 검사를 추가했다.

# 변경 파일

- `Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap`
- `Scripts/ConfigureSingijeonEnemyWave.py`
- `Scripts/VerifySingijeonEnemyWave.py`
- `Scripts/InspectSingijeonEnemyVisibility.py`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 주요 결정 사항

- Wave Actor Transform은 수정하지 않고 런타임에 생성되는 병사 메시 배율만 변경했다.
- 기존 OpenXR 표시 안정성을 위한 독립 `EnemySoldierActor` 구조는 변경하지 않았다.

# 테스트 결과

- 레벨 설정 검증: 성공
- 런타임 적군 Actor 45명 생성 확인
- 45명 메시 `Visible=true`, `HiddenInGame=false` 확인
- 대표 병사 월드 스케일이 약 `1.58~1.67`에서 `2.37~2.50`으로 정확히 1.5배 증가
- Wave 위치 `(-5981.444916, -3349.213945, 82.182526)` 및 대표 병사 생성 위치가 변경 전후 동일

# 남은 문제

- 커맨드렛은 Null RHI 계측이므로 Quest 3 헤드셋의 최종 양안 화면은 VR Preview에서 한 번 확인해야 한다.
