# L_Main 남한산성 Landscape 이식 계획

## 목표

`/Game/Namhansanseong/Maps/Demo_Namhansanseong`의 Landscape 지형과 Landscape Material/Layer 참조를 `/Game/Maps/Main/L_Main`에 이식한다. 기존 Main 교육 Manager, 이동 Trigger, PlayerStart는 유지한다.

## 확인 사항

- 원본은 Landscape 1개와 Landscape Streaming Proxy 121개로 구성된 World Partition 지형이다.
- 원본 지형은 `/Game/Namhansanseong/Materials/Landscape/MI_Landscape`를 사용한다.
- 기존 `L_Main`에는 64 Component의 평면 Landscape 1개가 있다.

## 구현 순서

1. `L_Main` 복제본에 원본 Landscape 액터 전체를 복사해 프록시/GUID/레이어 참조 보존 여부를 검증한다.
2. 검증 성공 시 기존 평면 Landscape만 제거하고 동일한 방식으로 원본 Landscape를 `L_Main`에 이식한다.
3. 기존 Main 교육 액터가 유지되는지, Landscape 122개와 Material 참조가 정상인지 재로드 검증한다.
4. Main 상태 문서와 완료 기록을 갱신한다.

## 비범위

- 남한산성 Demo의 성벽, 조명, 게임플레이 Blueprint, Foliage 액터는 복사하지 않는다.
- 원본 Demo 맵과 원본 Landscape Asset은 수정하지 않는다.

