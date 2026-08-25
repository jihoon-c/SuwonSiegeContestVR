# 신기전 장전 화살 에디터 메시 프리뷰 계획

상태: 완료 (2026-08-24)

## 목적

`BP_SingijeonHwacha`의 6 x 15 자동 장전 화살을 PIE 실행 전 에디터 뷰포트에서 실제 화살 메시로 확인한다.

## 현재 상태

- 런타임 `AutoLoadedArrowInstances`는 첫 화살이 장전된 뒤에만 89개 Instance를 생성한다.
- 에디터에서는 배열 중앙/방향을 나타내는 `AmmoGridCenterArrow`만 존재해 전체 배치를 확인할 수 없다.

## 구현 범위

1. 화차에 EditorOnly `AmmoGridEditorPreview` ISM 컴포넌트를 추가한다.
2. `OnConstruction`에서 `DefaultAmmoSlot`, `AutoFillGridOffset`, 행/열/간격 설정으로 90발을 생성한다.
3. `AutoFillArrowMesh`와 `AutoFillArrowMaterial`을 런타임 ISM과 동일하게 적용한다.
4. 프리뷰는 충돌, 그림자, 내비게이션 영향을 끄고 게임에서는 숨긴다.
5. 에디터 표시 여부를 Actor Details에서 끌 수 있게 한다.

## 변경 파일

- `SingijeonHwachaActor.h/.cpp`
- `SingijeonHwachaTests.cpp`
- `Scripts/VerifySingijeonAmmoCenterArrowAndFuseCord.py`
- `docs/Singijeon/specs/VR_INTERACTION.md`

## 구현 단계

1. EditorOnly ISM 기본 컴포넌트를 생성한다.
2. 화차 Construction 시 6 x 15 배열을 다시 계산한다.
3. PIE 시작 시 프리뷰 Instance를 비운다.
4. C++ 자동화 테스트와 레벨 검증 스크립트를 확장한다.

## 다른 Feature에 미치는 영향

- `GF_Singijeon`의 화차 편집 시각화에만 영향을 준다.
- Core, Shared Gameplay, 다른 Game Feature의 의존성과 런타임 흐름은 변경하지 않는다.

## 검증 방법

- 에디터 자동화 테스트: 프리뷰 컴포넌트, 메시, 90개 Instance, 비게임 렌더링 속성을 확인한다.
- `LV_Singijeon` 검증 스크립트: 배치 화차의 프리뷰와 실제 자동 장전 설정 일치를 확인한다.
- `SuwonSiegeContestVR.GF_Singijeon` 테스트 전체를 실행한다.

## 문서 반영

- `docs/Singijeon/specs/VR_INTERACTION.md`에 편집 방법을 추가한다.
- 완료 후 `docs/Singijeon/completed/`에 구현 및 검증 결과를 기록한다.
