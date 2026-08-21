# 작업

사적 남한산성 성벽 원본 Unreal 에셋 임포트

# 구현 내용

- `Stone_Barrier.fbx`를 결합 Static Mesh `SM_Stone_Barrier`로 임포트했다.
- a/b 세트의 Base Color, Normal, Roughness 텍스처 6개를 임포트했다.
- Normal과 Roughness 텍스처는 sRGB를 끄고 각각 Normalmap/Mask 압축으로 설정했다.
- `M_Stone_Barrier01a`, `M_Stone_Barrier01b`를 생성해 Base Color, Normal, Roughness를 연결했다.
- Static Mesh의 두 Material Slot에 각 머티리얼을 지정했다.

# 변경 파일

```text
Plugins/GameFeatures/GF_Singijeon/Content/Asset/NamhansanseongWall/*
Scripts/ImportNamhansanseongWall.py
Scripts/VerifyNamhansanseongWall.py
docs/Singijeon/plans/NAMHANSANSEONG_WALL_IMPORT.md
```

# 주요 결정 사항

- 신기전 전용 배경 에셋이므로 `GF_Singijeon` Content에 배치했다.
- 원본 FBX의 오브젝트를 하나의 Static Mesh로 결합하되 두 Material Slot은 유지했다.
- 원본 폴더는 Reimport Source로 유지했다.

# 테스트 결과

- Unreal Python Commandlet 임포트 성공
- Static Mesh 1개, Material 2개, Texture 6개 로드 성공
- 모든 Material Slot 할당과 a/b 머티리얼 사용 확인
- 검증 Commandlet 0 errors

# 남은 문제

- 원본 FBX에 퇴화 탄젠트와 거의 0인 바이노멀이 있어 임포트 경고가 발생한다. 에셋 사용은 가능하며 실제 조명에서 셰이딩 문제가 보이면 원본 메시 탄젠트 정리가 필요하다.
