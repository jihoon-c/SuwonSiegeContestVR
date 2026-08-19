# 작업

화차·신기전 플레이용 Blueprint 생성, 레벨 배치 및 Scenario Interaction 연결

# 구현 내용

- `BP_SingijeonHwacha`: `Wooden_Rocket_Cart` 메시, 탄약 슬롯·도화선 구성
- `BP_SingijeonArrow`: 플레이 검증용 메시와 기존 `BP_GrabComponent`
- `BP_SingijeonTorch`: 플레이 검증용 메시, 점화 영역과 기존 `BP_GrabComponent`
- `LV_Singijeon`: PlayerStart, 화차, 탄약, 횃불 배치
- `NA_02` 이후 Grab → 장전 → 횃불 Grab → 점화 → 발사 Interaction 연결
- Pawn Grab과 화차 장전·점화·발사 완료를 Scenario TargetID로 자동 보고

# 변경 파일

- `Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/*`
- `Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap`
- `Content/Data/DA_Scene_Singijeon.uasset`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*`
- `Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp`

# 테스트 결과

- Win64 Development 빌드 성공
- Core·GF_Singijeon Editor suffix 모듈 빌드 성공
- 생성 BP 3개 컴파일 성공
- Blueprint 전체 검사: 0 errors, 0 warnings, 0 failed loads
- BP GrabPoint, Scenario 순서, Level 배치 자동 검증 성공

# 남은 문제

- 신기전 탄약과 횃불의 전용 Static Mesh가 없어 기본 메시를 사용한다.
- 실제 VR 컨트롤러 장전·점화·발사는 에디터 재시작 후 VR Preview 실기 검증이 필요하다.
- 표적, VFX, SFX, Damage 연동은 후속 작업이다.
