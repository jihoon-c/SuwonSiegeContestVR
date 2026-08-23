# 작업

공심돈 보고·퇴각 적 사격 단계를 외부 UI나 디버그 Skip 없이 VR로 완주 가능하게 보완했다.

# 구현 내용

- `Gongsimdon_Report`에 보이는 원통형 `VRGrab` 보고 버튼을 추가했다.
- 보고 단계에서 버튼을 트리거로 누르면 작성된 정답인 동쪽·6명 보고를 제출한다.
- `AGongsimdonDefenseWeaponActor`를 추가하고 PlayerStart 근처에 배치했다.
- 손 컨트롤러 방향으로 50m Sweep 후 Health 대상 조준 보조를 적용하고 표준 Damage를 전달한다.
- Combat 가이드는 가까운 방어 사격 장치 위에 표시된다.

# 변경 파일

- `Plugins/GameFeatures/GF_Gongsimdon/Source/GF_Gongsimdon/Interaction/*`
- `Plugins/GameFeatures/GF_Gongsimdon/Content/Maps/LV_Gongsimdon.umap`
- `Plugins/GameFeatures/GF_Gongsimdon/Content/Data/DA_Scenario_Gongsimdon.uasset`
- 공심돈 구성·검증 스크립트와 문서

# 주요 결정 사항

- 음성 인식은 선택 확장으로 유지하고 기본 VR 입력만으로 체험을 끝낼 수 있게 했다.
- 특정 Enemy 클래스를 검사하지 않고 Shared Health Component를 가진 대상을 판정한다.

# 테스트 결과

- Editor 빌드 성공
- 공심돈 Action/Enemy/Scenario 자동화 통과
- 레벨 보고 버튼·방어 무기·Target ID 검증 통과

# 남은 문제

- 임시 기본 메시를 역사 무기/보고 장치 에셋으로 교체해야 한다.
- Quest 3에서 조준 보조 반경과 장치 배치 높이를 체감 조정해야 한다.
