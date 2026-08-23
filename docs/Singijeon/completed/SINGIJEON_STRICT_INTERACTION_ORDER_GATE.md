# 작업

신기전 물리 인터랙션의 Scenario 순서 강제

# 구현 내용

- `Hwacha_Load`가 현재 단계가 아니면 화살 슬롯이 Attach/자동 장전을 수행하지 않는다.
- `Torch_Ignite`가 현재 단계가 아니면 FirePit이 횃불 상태를 바꾸지 않는다.
- `Hwacha_Fuse`가 현재 단계가 아니면 Fuse 타이머와 VFX가 시작되지 않는다.
- `Hwacha_Fire` 단계가 아니면 Volley를 시작하지 않는다.
- 정상 단계에서는 기존 자동 완료 보고와 다음 단계 진행을 유지한다.

# 변경 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Singijeon/SingijeonAmmoSlotComponent.cpp
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Singijeon/FuseIgnitionComponent.cpp
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Singijeon/SingijeonHwachaActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Interaction/FirePitActor.cpp
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonHwachaTests.cpp
```

# 주요 결정 사항

- 조기 행동을 완료 기록만 무시하는 것이 아니라 물리 상태 변경 전부터 차단한다.
- 거절된 화살/횃불은 현재 위치에 남으므로 올바른 단계에서 다시 접촉해야 한다.

# 테스트 결과

- 조기 장전 거절 및 빈 슬롯 유지
- 조기 FirePit 점화 거절 및 횃불 꺼짐 유지
- 장전 후 조기 Fuse 점화 거절
- 각 올바른 단계에서 장전·FirePit·Fuse 정상 성공
- 프로젝트 자동화 16개 전체 성공

# 남은 문제

- Quest 3 VR Preview에서 거절된 물체를 뺐다가 다시 넣는 실제 조작감 확인
