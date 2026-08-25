# Scenario System

## 목적과 책임 경계

Core Scenario System은 **한 Level 안에서** `Interaction → Scene → Scenario` 진행을 관리한다.

- `UScenarioManagerComponent`: 현재 Scene/Interaction, 상태, 분기, 지연, 재시작과 디버그 이동
- `FScenarioStageDefinition`: Scenario 내부에 인라인으로 저장되는 Stage와 Interaction 목록
- `UScenarioDefinition`: 전체 Stage 흐름, 시작 Stage, Narration Table
- `UScenarioInteractableComponent`: Actor가 `TargetID + InteractionType` 사건을 보고하는 통로
- `UScenarioObservationComponent`: HMD 시야각·거리·시선 유지 시간·가시선 판정
- `UScenarioNarrationBridgeComponent`: 기존 `UNarrationSequenceComponent`와 Scenario를 연결

다음 책임은 Scenario System에 넣지 않는다.

- Level Travel과 Level을 넘어 유지되는 진행도: `ExperienceSubsystem`
- 음성·자막 재생: 기존 `UNarrationSequenceComponent`
- 신기전 발사·장전·점화 등 체험 고유 동작: `GF_Singijeon`

## 콘텐츠 생성 순서

1. `ScenarioDefinition` Data Asset `DA_Scenario_*`를 만든다.
2. `Stages` 배열에 Stage를 추가하고 `StageID`, `StartInteractionID`, `Interactions`를 입력한다.
3. `StartStageID`를 지정하고 Narration 사용 시 같은 Asset의 `NarrationTable`을 지정한다.
4. `DA_Experience_*`의 `ScenarioDefinition`에 `DA_Scenario_*`를 지정한다.
5. Level의 `BP_ScenarioManager`에 `ExperienceDefinition`을 지정한다.
6. Level별 나레이션 DT가 필요하면 Manager의 `Level Narration Table`을 지정한다. 비워 두면 Scenario의 `NarrationTable`을 사용한다.

별도 `DA_Scene_*`를 만들거나 Level Manager에 Scenario를 다시 지정하지 않는다. `Level Narration Table`은 의도적인 레벨별 Override이고, 최종 사용 DT는 Manager의 읽기 전용 `Resolved Configuration > Narration Table`에서 확인한다.

Stage 배열 순서는 편집 가독성과 복귀 시 이전 단계 완료 상태에 사용한다. 실제 분기는 `StartInteractionID`, `NextInteractionID`, `SuccessInteractionID`, `FailInteractionID`, `NextStageID`가 결정한다.

### Interaction 배열 편집 화면

배열 항목 제목에는 `InteractionID`가 표시된다. `InteractionType`에 따라 관련 필드만 노출된다.

- `Narration`: `NarrationID` 표시, `TargetID`와 성공/실패 분기 숨김
- `Objective`: `ObjectiveText`와 즉시 완료 옵션 표시
- `Wait`: `Duration` 표시
- Grab 등 실제 Interaction: `TargetID`와 성공/실패 분기 표시

실제 Interaction에는 `GuideAction`과 선택적인 `GuideText`가 함께 표시된다. `Auto`는 Interaction Type에서 기본 입력 안내를 결정하고, `Custom`처럼 타입만으로 실제 조작을 알 수 없는 단계는 `Grab`, `Drag`, `Trigger`, `Observe`, `Combat` 등을 명시한다. `Hidden`은 플레이어 입력 없이 자동 진행되는 단계에 사용한다.

이는 Details 패널 표시만 변경하며 기존 Scenario 데이터와 런타임 흐름에는 영향을 주지 않는다.

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

## 인터랙션 가이드

`AScenarioManagerActor.InteractionGuide`는 현재 Interaction이 시작되면 동일한 `TargetID`와 `InteractionType`을 지원하는 `ScenarioInteractableComponent`를 찾는다. 찾은 Interactor의 Guide Anchor에 입력 종류와 안내 문구가 담긴 World Widget을 표시하고 완료·실패·Skip 시 즉시 숨긴다.

각 `ScenarioInteractableComponent`는 실행 전 에디터에서 노란 화살표로 Guide Anchor를 표시한다. 액터를 선택한 뒤 화살표를 클릭하고 이동 기즈모로 끌면 결과가 `Scenario > Guide > Guide Anchor Offset`에 저장된다. 숫자로도 Actor Bounds 상단 기준 위치를 조절할 수 있으며 기본 높이는 10cm다. `Show Guide Anchor In Editor`를 끄면 편집 마커만 숨겨지고 런타임 가이드에는 영향이 없다.

| GuideAction | 기본 표시 |
|---|---|
| `Grab` | `TRIGGER / 트리거를 눌러 잡기` |
| `Drag` | `HOLD + MOVE / 트리거를 누른 채 이동` |
| `Trigger` | `TRIGGER / 트리거로 작동` |
| `Observe` | `LOOK / 대상을 바라보기` |
| `Combat` | `AIM + TRIGGER / 조준하고 트리거로 발사` |

`GuideText`를 입력하면 아래 한국어 안내만 교체된다. 여러 Actor가 같은 Target ID를 사용하면 플레이어와 가장 가까운 Actor를 선택한다. 대상 Actor가 Feature/Level 로딩 순서 때문에 늦게 생성되면 활성 Interaction 동안 30Hz로 재탐색하여 생성 직후 표시한다. 표시 중인 가이드는 Player Pawn의 활성 HMD CameraComponent를 우선 기준으로 사용해 Yaw와 Pitch가 플레이어 시점을 향하며 Roll은 고정한다. 따라서 도화선처럼 시선보다 낮고 가까운 대상에서도 패널이 비스듬히 보이지 않는다. 나레이션, Objective, Wait, Sequence, Spawn은 기본적으로 대상 가이드를 표시하지 않는다.

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
   └─ Experience Definition = DA_Experience_Singijeon

DA_Experience_Singijeon
└─ Scenario Definition = DA_Scenario_Singijeon
   └─ Stages[Singijeon]
      └─ Interactions[NAR_01 ... NAR_21, INT_01 ... INT_07]

DA_Scenario_Singijeon.NarrationTable = DT_Narration
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

`RestartScenario`, `RestartScene`, `RestartInteraction`, `SkipCurrentInteraction`, `CompleteCurrentInteraction`, `GoToScene`, `GoToInteraction`, `PrintDebugState`를 Blueprint에서 호출할 수 있다. 기존 API의 `Scene` 명칭은 Blueprint 호환 때문에 유지하지만 신규 제작 UI와 문서에서는 `Stage`로 표시한다.

Data Asset은 Scenario 시작 시 다음 항목을 검증한다.

- 빈 ID와 중복 ID
- 존재하지 않는 Start ID
- 존재하지 않는 Next/Success/Fail ID
- 존재하지 않는 Next Scene ID

검증 실패 시 `OnValidationFailed`가 발생하고 Scenario는 `Failed`가 된다.

## Experience 왕복과 진행 복원

`ExperienceTravelTriggerActor.TriggerExperienceTravel` 또는 `AMainEducationScenarioManagerActor`는 이동 전에 복귀할 `ScenarioID`, `SceneID`, `InteractionID`를 세션 체크포인트로 저장한다. 목적지 Scenario가 완료되면 Experience의 `ReturnLevel`로 이동하고, Main Manager의 Experience Bridge가 체크포인트 이전 Interaction을 완료 상태로 복원한 뒤 지정 Interaction부터 재개한다.

현재 Main 교육 Asset은 첨부 교육 시나리오 순서에 맞춰 신기전 체험 후 주요 시설을 진행한다.

```text
L_Main / DA_Scenario_MainEducation
MAIN_INTRO.TRAVEL_SINGIJEON → 신기전 → AFTER_SINGIJEON
GONGSIMDON.TRAVEL_GONGSIMDON → 공심돈 → AFTER_GONGSIMDON
ONGSEONG.TRAVEL_ONGSEONG → 옹성 → AFTER_ONGSEONG
NOKRO.TRAVEL_NOKRO → 녹로 → AFTER_NOKRO
GEOJUNGGI.TRAVEL_GEOJUNGGI → 거중기 → AFTER_GEOJUNGGI
SUMMARY
```

Main 전용 Manager는 `Travel_*` Target의 Route를 찾고 현재 Interaction의 다음 ID를 복귀 지점으로 저장한다. Route Asset이 없으면 이동하지 않고 `OnExperienceUnavailable`을 발생시킨다. 현재 녹로와 거중기는 이 미연결 상태다.

기존 배치 Trigger 방식도 계속 지원한다. Trigger의 `RequiredInteractionID`는 현재 Main Interaction이 일치할 때만 이동을 허용하고, Pawn Overlap과 Player Camera(HMD) 위치 진입을 모두 지원한다.
