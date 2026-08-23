# VR 나레이션·자막 시스템 명세

## 구성

| 요소 | 역할 |
|---|---|
| `BP_VRPlayerPawn` | 프로젝트 전용 VR Camera/Controller와 시야 앞 HUD 소유 |
| `UNarrationSequenceComponent` | Data Table 행 로드, 음성 재생, 자막 수명, 다음 단계 진행 관리 |
| `USubtitleWidget` | 기본 자막 표시. 필요하면 Blueprint Widget으로 교체 가능 |
| `DT_Narration` | 나레이션, 자막, 다음 행, 종료 이벤트 및 후속 위젯 데이터 |

## DT_Narration 행 필드

`DT_Narration`의 Row Structure는 `NarrationSequenceRow`다.

| 필드 | 설명 |
|---|---|
| `SpeakerName` | 화자 이름. 비워두면 화자 줄을 숨긴다. |
| `Subtitle` | 나레이션과 함께 표시할 자막 |
| `NarrationSound` | 음성 Soft Reference. 현재 행이 재생될 때 비동기 로드한다. |
| `PreviewDuration` | 음성이 아직 없을 때 사용할 임시 자막 시간 |
| `NextRow` | 다음 행 이름. 행 순서가 아니라 명시적 연결을 사용한다. |
| `AdvanceMode` | `Auto`, `WaitForContinue`, `Stop` |
| `AdvanceDelay` | Auto 진행 전 추가 대기 시간 |
| `CompletionEvents` | 음성 종료 후 순서대로 Broadcast할 이벤트 이름 배열 |
| `PostNarrationWidgetClass` | 음성 종료 후 시야 앞 Event HUD에 표시할 Widget Class |

권장 Row Name은 `Main_Intro_001`, `Singijeon_Explain_010`처럼 범위와 순서를 함께 표현한다.

## 기본 실행

1. `BP_VRPlayerPawn`의 `NarrationSequence`에서 `Play Sequence`를 호출한다.
2. `Narration Table`에는 `DT_Narration`, `Start Row`에는 첫 행 이름을 전달한다.
3. 음성 로드가 끝나면 자막과 음성이 동시에 시작된다.
4. `AudioComponent.OnAudioFinished`에서 자막이 사라지고 후속 이벤트가 실행된다.
5. 행의 진행 정책에 따라 다음 행 재생, 외부 신호 대기 또는 시퀀스 종료로 이동한다.

음성 파일이 없는 제작 단계에는 `PreviewDuration` 동안 자막만 표시되므로 흐름을 먼저 테스트할 수 있다.

## 후속 이벤트 연동

`NarrationSequence.OnSequenceEvent(EventName, SourceRow)`에 Blueprint Manager가 바인딩한다. Core는 특정 Feature를 직접 참조하지 않는다.

예시 이벤트 이름:

- `Quiz.Show.SuwonHwaseong`
- `Experience.Open.Singijeon`
- `NPC.PlayGesture.PointLeft`
- `Tutorial.EnableInteraction.Hwacha`

한 행에서 여러 동작이 필요하면 `CompletionEvents` 배열에 여러 이름을 넣는다. 실제 동작은 Main Manager 또는 활성 Game Feature Manager가 처리한다.

## 후속 위젯 연동

`PostNarrationWidgetClass`를 지정하면 Pawn의 `NarrationEventHUD`에 World Space 위젯이 나타난다.

- 선택이나 확인이 필요한 위젯은 `AdvanceMode = WaitForContinue`로 설정한다.
- 위젯 작업 완료 시 Pawn의 `DismissNarrationWidget(true)`를 호출한다.
- 이 호출은 위젯을 숨기고 대기 중인 `NextRow`로 진행한다.
- 자동 진행 행에 위젯을 지정하면 다음 행 시작 시 위젯이 자동으로 정리되므로, 상호작용 위젯에는 사용하지 않는다.

## 제어 API

| 함수 | 용도 |
|---|---|
| `PlaySequence(Table, StartRow)` | 새 시퀀스 시작 |
| `PlayRow(RowName)` | 특정 행 즉시 재생 |
| `SkipCurrentNarration()` | 현재 음성을 종료 처리하고 행의 후속 동작 실행 |
| `ContinueSequence()` | `WaitForContinue` 상태에서 다음 행 진행 |
| `StopSequence()` | 음성·타이머·자막 정리 |
| `DismissNarrationWidget()` | Event HUD 정리 및 선택적으로 다음 행 진행 |

## VR HUD 원칙

자막은 Camera에 부착된 World Space Widget이며 기준 위치는 HMD 전방 85cm, 아래 28cm다. OpenXR HMD에서 누락될 수 있는 Screen Space WidgetComponent는 사용하지 않는다. 월드 메시가 사이에 들어오기 어려운 가까운 위치와 높은 Translucency Sort Priority를 사용하며, Blueprint의 `SubtitleHUDOffset`으로 위치를 조정할 수 있다.

## 현재 제한

- 새 Pawn은 Camera, Grip/Aim Motion Controller, Widget Interaction, Narration HUD와 Grab / Teleport / Snap Turn 입력을 제공한다.
- Teleport 목적지는 NavMesh 투영을 사용하므로 사용하는 Level에 유효한 NavMesh가 필요하다.
- OpenXR 런타임과 Android 스탠드얼론 실기기에서 입력 및 HUD 사용성 검증이 필요하다.
