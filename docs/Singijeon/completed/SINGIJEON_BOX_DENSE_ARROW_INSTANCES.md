# 작업

`BP_SingijeonBox` 내부 화살 ISM 밀도 보완.

# 구현 내용

- 기존 한 층 7개 화살을 상자 내부 Bounds에 맞춘 7열 x 6층, 총 42개 화살로 교체했다.
- 긴 화살이 상자 벽을 벗어나지 않도록 회전은 유지하고 X 위치만 작은 규칙적 편차를 적용했다.
- 밀집 ISM의 충돌, Overlap, 그림자를 비활성화해 VR 렌더링 비용을 제한했다.
- 기존 상자 Static Mesh의 위치와 스케일, 레벨 배치 Actor Transform은 변경하지 않았다.

# 변경 파일

- `/GF_Singijeon/Gameplay/BP_SingijeonBox`
- `Scripts/FillSingijeonBoxArrowInstances.py`
- `Scripts/InspectSingijeonBox.py`
- `Scripts/VerifySingijeonBoxArrowInstances.py`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 주요 결정 사항

- 상자 외곽 약 225 x 80 x 60cm와 화살 Bounds 약 200 x 12 x 9cm를 기준으로 7 x 6 배열을 사용했다.
- 별도 Actor 42개가 아닌 기존 ISM 하나를 유지했다.

# 테스트 결과

- 저장 후 Blueprint 재로드 및 Compile 성공.
- ISM Instance 42개, Y 범위 -16~44cm, Z 범위 0~40cm 검증 성공.
- 상자 컴포넌트 위치 `(-269.749206, 112, 0)`와 스케일 `(4.5, 2, 2)` 보존 확인.
- 충돌 및 그림자 비활성 확인.

# 남은 문제

- 없음.
