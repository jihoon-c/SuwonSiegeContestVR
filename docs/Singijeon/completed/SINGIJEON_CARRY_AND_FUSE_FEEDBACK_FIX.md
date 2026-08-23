# 작업

화차 한 손 드래그 정지 및 도화선 접촉 피드백 수정

# 구현 내용

- 운반 이동에 `Sweep Movement` 옵션을 추가하고 화차 직접 손 추종은 비활성화했다.
- 바닥과 접촉한 큰 화차 Root Collision 때문에 Sweep이 모든 이동량을 거부하던 문제를 제거했다.
- Fuse 접촉 반경을 12cm에서 24cm로 확대했다.
- 화차에 `FuseIgnitionEffect` Niagara 컴포넌트를 추가해 점화 시작·취소·완료와 동기화했다.
- `BP_SingijeonHwacha` 및 `LV_Singijeon` 배치본에 동일 설정을 저장했다.

# 변경 파일

```text
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Interaction/TwoHandCarryComponent.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Singijeon/FuseIgnitionComponent.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/*/Singijeon/SingijeonHwachaActor.*
Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Tests/SingijeonHwachaTests.cpp
Plugins/GameFeatures/GF_Singijeon/Content/Gameplay/BP_SingijeonHwacha.uasset
Content/Maps/LV_Singijeon.umap
Scripts/ConfigureSingijeonCarryFuse.py
Scripts/VerifySingijeonCarryFuse.py
```

# 주요 결정 사항

- 점화 순서는 유지한다. 횃불은 먼저 FirePit에서 활성화돼야 Fuse가 승인한다.
- 운반 Sweep은 삭제하지 않고 에디터 옵션으로 유지해 다른 충돌 구성에서는 다시 사용할 수 있게 했다.
- `docs/ARCHITECTURE.md`와 Core는 변경하지 않았다.

# 테스트 결과

- Win64 Development Editor 빌드 성공
- Blueprint/레벨 검증 스크립트 성공
- `GF_Singijeon` 자동화 3개 성공
  - 적군 Wave/Volley
  - 화차 장전·좌우 한 손 운반·Fuse VFX 시작/취소
  - FirePit → 횃불 → Fuse 순서

# 남은 문제

- Quest 3 VR Preview에서 실제 손잡이 접근감과 Fuse 불꽃 크기의 최종 시각 확인이 필요하다.
