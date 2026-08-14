# Scenario System

## 목적과 책임 경계

Core Scenario System은 **한 Level 안에서** `Interaction → Scene → Scenario` 진행을 관리한다.

- `UScenarioManagerComponent`: 현재 Scene/Interaction, 상태, 분기, 지연, 재시작과 디버그 이동
- `UScenarioSceneData`: 한 Scene의 Interaction 목록
- `UScenarioDefinition`: Scene 목록과 시작 Scene
- `UScenarioInteractableComponent`: Actor가 `TargetID + InteractionType` 사건을 보고하는 통로
- `UScenarioObservationComponent`: HMD 시야각·거리·시선 유지 시간·가시선 판정
- `UScenarioNarrationBridgeComponent`: 기존 `UNarrationSequenceComponent`와 Scenario를 연결

다음 책임은 Scenario System에 넣지 않는다.

- Level Travel과 Level을 넘어 유지되는 진행도: 향후 `ExperienceSubsystem`
- 음성·자막 재생: 기존 `UNarrationSequenceComponent`
- 신기전 발사·장전·점화 등 체험 고유 동작: `GF_Singijeon`

## 콘텐츠 생성 순서

1. Content Browser에서 `Miscellaneous > Data Asset`을 선택하고 `ScenarioSceneData`로 `DA_Scene_*`를 만든다.
2. 각 Scene에 고유 `SceneID`, `StartInteractionID`, `Interactions`를 입력한다.
3. `ScenarioDefinition`으로 `DA_Scenario_*`를 만들고 Scene 배열, `StartSceneID`를 지정한다.
4. Level에 `/Game/Core/Scenario/Managers/BP_ScenarioManager`를 배치한다.
5. 배치 Actor의 최상위 `Scenario > Configuration > Scenario Definition`에 `DA_Scenario_*`를 지정한다.
6. 같은 영역의 `Narration Table`에 기존 `DT_Narration`을 지정한다.
7. 자동 시작이면 `Auto Start Scenario`를 켠다. 수동 시작이면 `Start Configured Scenario`를 호출한다.

> `DA_Scene_*`는 Scene 하나이고 `Scenario Definition` 슬롯에 직접 연결할 수 없다. 반드시 `ScenarioDefinition` 타입의 `DA_Scenario_*`를 만들고 그 `Scenes` 배열에 `DA_Scene_*`를 넣는다.

배열 인덱스는 흐름을 결정하지 않는다. `StartInteractionID`, `NextInteractionID`, `SuccessInteractionID`, `FailInteractionID`가 실제 흐름을 결정한다. 모든 ID는 해당 Data Asset 안에서 유일해야 한다.

## Interaction 타입 연결

| 타입 | 권장 처리 |
|---|---|
| `Narration` | `NarrationID`를 `DT_Narration` Row Name으로 입력. 재생 종료 시 Bridge가 자동 완료 |
| `Objective` | `OnObjectiveRequested`에서 UI 표시. 즉시 단계면 `bCompleteOnStart=true` |
| `Wait` | `Duration` 후 자동 완료 |
| `Grab`, `Press`, `Trigger`, `Combat` | 대상 Actor의 `ScenarioInteractableComponent.ReportCompleted` 호출 |
| `Observe` | 대상 Actor의 `ScenarioObservationComponent.StartObservation` 호출 |
| `Quiz`, `VoiceCommand` | 외부 시스템이 결과를 `ReportInteractionResult(TargetID, Type, Success)`로 보고 |
| `Sequence`, `Spawn`, `Custom` | `OnInteractionRequested`에서 Blueprint 연출 후 완료/실패 보고 |

`OnInteractionRequested`는 모든 Interaction에 발생한다. 전용 이벤트인 `OnNarrationRequested`, `OnObjectiveRequested`도 함께 사용할 수 있다.

## Actor 연결 예시

망원경 Actor에 `ScenarioInteractableComponent`를 추가한다.

```text
TargetID = Telescope_01
SupportedInteractionTypes = [Grab]
```

기존 Grab 성공 이벤트에서 해당 컴포넌트의 `ReportCompleted(Grab)`을 호출한다. Actor는 다음 Scene이나 내레이션을 직접 실행하지 않는다. 현재 단계의 Type과 TargetID가 일치할 때만 Manager가 완료로 해석한다.

## Observation 설정

관찰 대상 Actor에 `ScenarioObservationComponent`를 추가하고 다음을 지정한다.

- `TargetID`: Scenario Interaction의 TargetID와 동일
- `ObservationTarget`: 판정 중심 Scene Component. 비어 있으면 Owner Root 사용
- `RequiredViewTime`: 연속 응시 시간
- `RequiredViewAngle`: HMD 정면과 대상 방향 사이 허용 각도
- `MaxDistance`: 최대 거리
- `bRequireLineOfSight`: 장애물 차단 검사 여부

`StartObservation()` 이후에만 Tick이 켜지며 완료/중지 후 다시 꺼진다.

## 내레이션 연결

기존 `DT_Narration` Row Name과 Interaction의 `NarrationID`를 동일하게 둔다. Bridge는 플레이어 Pawn의 `UNarrationSequenceComponent.PlaySequence(DT, NarrationID)`를 호출하고 `OnSequenceFinished`에서 현재 Scenario Interaction을 완료한다.

Bridge의 `NarrationTable`을 비워 두면 아무 동작도 하지 않으므로 `OnNarrationRequested`를 Blueprint에서 직접 처리할 수도 있다. 별도 Narration Manager를 추가하지 않는다.

### 현재 신기전 연결

```text
LV_Singijeon
└─ BP_ScenarioManager
   ├─ Scenario Definition = DA_Scenario_Singijeon
   ├─ Narration Table = DT_Narration
   └─ Auto Start Scenario = true

DA_Scenario_Singijeon
└─ StartSceneID = Singijeon
   └─ DA_Scene_Singijeon
      └─ Interaction "singijeon"
         ├─ Type = Narration
         └─ NarrationID = NewRow
```

### 나레이션 후 이벤트 연결

`DT_Narration` Row의 `CompletionEvents` 배열에 의미 있는 이름을 입력한다. 예를 들어 `EnableHwacha`, `StartIgnitionGuide`를 넣는다.

`BP_SingijeonExperienceManager` 또는 해당 Feature Manager의 BeginPlay에서 다음 순서로 바인딩한다.

```text
Get Player Pawn
→ Cast to BP_VRPlayerPawn
→ Get Narration Sequence
→ Bind Event to OnSequenceEvent
→ EventName으로 Switch on Name
```

`OnSequenceEvent`는 `(EventName, SourceRow)`를 전달한다. Feature Manager는 EventName을 해석해 신기전 Actor 활성화, 안내 연출, Niagara 등을 실행한다. 나레이션 Interaction 자체의 완료와 다음 Interaction 이동은 Bridge가 `OnSequenceFinished`를 받아 자동 처리하므로, 같은 이벤트에서 `CompleteInteraction`을 다시 호출하지 않는다.

개별 Row 종료만 필요하면 `OnNarrationFinished(RowName)`, 연결된 Row 전체 종료가 필요하면 `OnSequenceFinished`, 자막 변경은 `OnSubtitleChanged`에 바인딩한다.

## 디버그

`RestartScenario`, `RestartScene`, `RestartInteraction`, `SkipCurrentInteraction`, `CompleteCurrentInteraction`, `GoToScene`, `GoToInteraction`, `PrintDebugState`를 Blueprint에서 호출할 수 있다. `GetDebugSnapshot`은 Scenario/Scene/Interaction ID, Type, Target, 상태를 반환한다.

Data Asset은 Scenario 시작 시 다음 항목을 검증한다.

- 빈 ID와 중복 ID
- 존재하지 않는 Start ID
- 존재하지 않는 Next/Success/Fail ID
- 존재하지 않는 Next Scene ID

검증 실패 시 `OnValidationFailed`가 발생하고 Scenario는 `Failed`가 된다.
