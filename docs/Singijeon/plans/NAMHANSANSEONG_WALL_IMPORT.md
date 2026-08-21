# 목적

상태: 완료

`사적_남한산성_성벽_원본`의 FBX와 텍스처를 Unreal에서 바로 사용할 수 있는 신기전 Feature 에셋으로 임포트한다.

# 현재 상태

- 원본 폴더에 `Stone_Barrier.fbx`와 a/b 세트의 BC, NM, RN PNG가 있다.
- Unreal Content 안에는 해당 원본을 사용하는 에셋이 없다.

# 구현 범위

- FBX를 Static Mesh로 임포트
- BC/NM/RN 텍스처 임포트 및 용도별 설정
- 메시 Material Slot에 사용할 머티리얼 생성 및 텍스처 연결
- 임포트 결과와 참조 상태 검증

# 변경 예정 파일

```text
Plugins/GameFeatures/GF_Singijeon/Content/Asset/NamhansanseongWall/*
Scripts/ImportNamhansanseongWall.py
Scripts/VerifyNamhansanseongWall.py
docs/Singijeon/completed/NAMHANSANSEONG_WALL_IMPORT.md
```

# 구현 단계

1. 원본 파일과 머티리얼 슬롯 조사
2. 텍스처와 FBX 임포트
3. 머티리얼 생성 및 메시 슬롯 연결
4. 에셋 저장, 재조회, 오류 검증

# 다른 Feature에 미치는 영향

- `GF_Singijeon` Content에만 에셋을 추가하며 Core와 다른 Feature는 수정하지 않는다.

# 검증 방법

- Python Commandlet 임포트 성공 여부
- Static Mesh, 텍스처, 머티리얼 로드 및 참조 재조회
- 메시 Material Slot이 생성 머티리얼을 사용하는지 확인

# 결과

- `SM_Stone_Barrier`와 a/b 머티리얼 2개, 텍스처 6개를 임포트했다.
- 메시의 두 Material Slot에 a/b 머티리얼을 연결했다.
- 저장본 재조회 검증을 통과했다.
