# 작업

`DA_Scene_Singijeon` 신기전 체험 Interaction 흐름 구성

# 구현 내용

- 기존 `NA_01 -> NA_02` 나레이션 흐름 뒤에 `INT_01 -> INT_07` 체험 흐름을 연결했다.
- `INT_01`: 무기 집기 (`Grab`, `Singijeon_Ammo`)
- `INT_02`: 신기전 화차 배치 (`Custom`, `Hwacha_Load`)
- `INT_03`: 신기전 조준·위치 이동 (`Custom`, `Hwacha_Aim`)
- `INT_04`: 횃불 집기 (`Grab`, `Singijeon_Torch`)
- `INT_05`: 횃불 점화 (`Trigger`, `Torch_Ignite`)
- `INT_06`: 도화선 점화 (`Trigger`, `Hwacha_Fuse`)
- `INT_07`: 신기전 발사 완료 (`Combat`, `Hwacha_Fire`)
- 실패한 Interaction은 동일 ID로 재시도하도록 `FailInteractionID`를 지정했다.

# 변경 파일

- `Content/Data/DA_Scene_Singijeon.uasset`

# 테스트 결과

- 에셋 저장 후 Interaction ID, Type, TargetID, NextInteractionID, FailInteractionID를 재조회했다.
- `INT_01`부터 `INT_07`까지 순차 연결과 실패 재시도 연결을 확인했다.

# 잔여 문제

- `Hwacha_Aim`과 `Torch_Ignite`는 현재 액터 코드에 완료 보고자가 없으므로 실제 진행을 위해 별도 완료 이벤트 연결이 필요하다.
