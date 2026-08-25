# 작업

신기전 화차의 자동 장전 6 x 15 화살 배열을 게임 실행 전에 실제 화살 메시로 확인할 수 있게 했다.

# 구현 내용

- `AmmoGridEditorPreview` EditorOnly ISM 컴포넌트를 추가했다.
- 화차 Construction 시 `DefaultAmmoSlot`, `AutoFillGridOffset`, 행/열/간격 설정으로 90발 전체를 생성한다.
- `AutoFillArrowMesh`와 `AutoFillArrowMaterial`을 에디터 프리뷰에도 적용한다.
- `Show Ammo Grid Preview In Editor`로 표시 여부를 선택할 수 있다.
- 충돌, 그림자, 내비게이션 영향을 비활성화하고 PIE 시작 시 프리뷰 Instance를 제거한다.

# 변경 파일

- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Public/Singijeon/SingijeonHwachaActor.h`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Singijeon/SingijeonHwachaActor.cpp`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonHwachaTests.cpp`
- `Scripts/VerifySingijeonAmmoCenterArrowAndFuseCord.py`
- `docs/Singijeon/specs/VR_INTERACTION.md`

# 주요 결정 사항

- 런타임 89개 Instance와 별개인 EditorOnly 컴포넌트를 사용해 플레이/패키징 비용을 분리했다.
- 첫 물리 화살까지 포함한 90발 전체를 프리뷰해 최종 장전 형태를 그대로 보여 준다.
- 프리뷰 Transform 계산은 런타임 자동 장전과 동일한 축/간격 규칙을 사용한다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development`: 성공
- `SuwonSiegeContestVR.GF_Singijeon`: 7/7 성공
- `LV_Singijeon` 검증: 프리뷰 90개, 런타임과 동일 메시, 게임 숨김/무충돌/무그림자 확인

# 남은 문제

- 없음.
