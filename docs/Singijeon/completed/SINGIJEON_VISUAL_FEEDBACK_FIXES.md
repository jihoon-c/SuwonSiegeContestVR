# 작업

신기전 화로 불꽃 높이, 자동 장전 화살 머티리얼, 양손 화차 운반 손잡이 하이라이트 보완

# 구현 내용

- `BP_SingijeonFirePit.IgnitionArea`는 화로 상단 `Z=102`로 이동했다. `NS_Fire`의 대칭 바운드 때문에 시각 하단이 내려가는 현상을 보정하여 FireEffect 원점은 `Z=155`, 스케일은 `0.55`로 설정했다.
- 화차의 `Auto Fill Arrow Material`에 `M_Arrow01b`를 지정했다.
- 런타임 자동 장전 시 명시적 머티리얼이 없으면 첫 물리 화살의 모든 머티리얼 슬롯을 ISM으로 복사한다.
- 화차에 `LeftHandleHighlight`, `RightHandleHighlight`와 `AimScenarioInteractor (Hwacha_Aim)`를 추가했다.
- 장전 완료 후 손잡이 안내를 켜고, 양손을 올바르게 잡는 동안 숨기며, 이동을 완료하지 않고 놓으면 다시 표시한다.
- 최초 장전 위치에서 30cm 이동 또는 10도 회전하면 `Hwacha_Aim`을 성공 보고하고 안내를 최종 해제한다.
- 외부 목표 지점에서 명시적으로 완료해야 하는 경우 `CompleteAimInteraction()`을 호출할 수 있다.
- 하이라이트는 충돌과 그림자가 없는 시각 전용 프록시이며 Blueprint에서 위치, 크기, 머티리얼을 조정할 수 있다.

# 변경 파일

- `ASingijeonHwachaActor` 헤더/구현
- `SingijeonHwachaTests.cpp`
- `BP_SingijeonFirePit`, `BP_SingijeonHwacha`, `BP_SingijeonArrow`, `LV_Singijeon`
- `ConfigureSingijeonFirePitFlow.py`, `ConfigureSingijeonAutoFillArrowGrid.py`
- `ConfigureSingijeonHandleHighlights.py`, `VerifySingijeonVisualFeedback.py`

# 주요 결정 사항

- 화차 본체 메시가 한 개의 머티리얼 슬롯만 가지므로 본체 전체 Outline 대신 손잡이 위치에만 겹치는 별도 프록시를 사용했다.
- 손잡이 하이라이트는 완료 효과가 아니라 다음 행동을 알려주는 가이드다. 한 손만 잡은 상태에는 유지되고 양손 운반이 확정되면 숨는다.
- 양손 운반이 중단돼도 `Hwacha_Aim`이 미완료라면 다시 표시한다.

# 테스트 결과

- `SuwonSiegeContestVREditor Win64 Development`: 성공
- `VerifySingijeonVisualFeedback.py`: 성공
- `SuwonSiegeContestVR.GF_Singijeon`: 2/2 성공(Grab/Drop/이동 완료 가이드 상태 포함)

# 남은 문제

- 실제 HMD에서 손잡이 프록시가 원본 메시와 정확히 겹치는지 최종 시각 확인이 필요하다. 필요하면 Blueprint의 좌우 Highlight 컴포넌트 Transform만 미세 조정하면 된다.
