# 작업

Enemy Wave 편대를 에디터에서 미리 표시하고 횃불을 눕힐 때 화염이 손잡이 전체에 퍼지는 문제를 수정했다.

# 구현 내용

- `Singijeon_EnemyWave`가 에디터 월드에서 `EnemyPreview_01~45` Skeletal Mesh Component를 생성한다.
- 프리뷰는 실제 Formation Seed, Route, 크기와 LOD를 사용하며 충돌·Tick·그림자를 사용하지 않는다.
- 프리뷰는 Transient/EditorOnly이므로 저장·패키징되지 않고 BeginPlay에서 제거된다.
- `Show Enemy Preview In Editor`로 표시 여부를 변경할 수 있다.
- 횃불의 기존 `NS_Fire`는 Static Mesh 전체를 샘플링하므로 런타임에서 비활성화한다.
- `IgnitionArea`에 부착된 `TorchFlameEffect` 점 발화 효과만 점화 상태에 따라 활성화한다.
- 화로와 화차 도화선의 Niagara 효과는 변경하지 않았다.

# 변경 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `IgnitionSourceActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`, `SingijeonHwachaTests.cpp`
- `InspectSingijeonEnemyVisibility.py`, `VerifySingijeonFirePitFlow.py`

# 주요 결정 사항

에디터 프리뷰는 애니메이션을 평가하지 않는 Ref Pose로 표시해 편대 배치 확인 비용을 줄인다. 횃불은 메시 표면 샘플링 효과를 축소하지 않고 점 발화 효과로 분리해 회전 방향과 관계없이 발화 위치를 제한한다.

# 테스트 결과

- Editor 빌드 성공
- GF_Singijeon 자동화 테스트 7/7 성공
- `LV_Singijeon` 로드 시 Editor Preview 45개 확인
- TorchFlameEffect가 `IgnitionArea`에 부착되고 기존 Niagara가 재활성화되지 않는 것 확인

# 남은 문제

- 실제 VR에서 불꽃 크기는 `TorchFlameEffect.Relative Scale`로 최종 미술 조정할 수 있다.
