# 완료 기록 — 키워드 감지 마이크 테스트 레벨

**완료일**: 2026-08-27
**계층**: Core (Debug)
**계획**: `docs/Core/plans/2026-08-27_VOICE_KEYWORD_TEST_LEVEL.md`
**선행 작업**: `docs/Core/completed/2026-08-27_SHERPA_ONNX_BACKEND.md`

---

# 작업

퀴즈/시나리오와 무관하게, 노트북 기본 마이크로 sherpa-onnx 백엔드가 인식하는 문장과
"옹성"·"신기전" 키워드 감지 여부를 화면 위젯으로 바로 확인할 수 있는 독립 테스트 레벨을 추가했다.

# 구현 내용

## Core C++ (`Core/Debug`)

* `UVoiceKeywordTestWidget : UUserWidget` — 상태 / 감지된 키워드 / 마지막 인식 문장 / 최근 기록을
  보여주는 네이티브 HUD 위젯. `RebuildWidget()`으로 트리를 직접 구성해 WBP 디자이너 없이 동작한다
  (`UInitialConsonantQuizWidget` 패턴 재사용).
* `AVoiceKeywordTestActor : AActor` — `USherpaVoiceRecognitionComponent`를 소유하고
  * `BeginPlay`에서 위젯을 뷰포트에 추가
  * 모델 로딩이 끝나는 대로(0.5초 간격 재시도) `Keywords=["옹성","신기전"]`, `ListenDuration=0`으로
    리스닝을 시작
  * `UVoiceRecognitionComponent`는 발화 1건마다 요청을 종료하는 단발 구조이므로,
    `OnRecognitionResult`를 받을 때마다 위젯을 갱신하고 **즉시 재리스닝**해 끊김 없이 반복시킨다
  * `OnVoiceStateChanged`로 모델 로딩/`ModelMissing`/오류 상태를 `GetBackendDescription()`과 함께
    위젯에 노출

기존 `UVoiceRecognitionComponent`/`USherpaVoiceRecognitionComponent` API는 수정하지 않았다.
전역 콘솔 명령 `ssv.voice.submit <텍스트>`는 리스닝 중인 아무 레코그나이저에나 전달되므로
마이크 없이도 이 액터를 대상으로 그대로 동작한다.

## 레벨

* `Content/Maps/Debug/L_VoiceKeywordTest.umap` — `L_XRTemplate`(바닥+조명만 있는 템플릿)을
  복제해 만든 최소 레벨.
* World Settings의 GameMode Override를 `ADebugFreeCameraGameMode`로 지정해 VR HMD/Pawn 없이
  PIE로 바로 진입한다 (`docs/OngseongCrossbow/plans/2026-08-24_NON_VR_COMBAT_TEST_GAMEMODE.md`에서
  만든 Core 디버그 GameMode 재사용, 신규 코드 없음).
* `AVoiceKeywordTestActor` 인스턴스 1개(`VoiceKeywordTestManager`) 배치.
* 레벨 조립(복제·GameMode 지정·액터 배치·저장·PIE 검증)은 언리얼 MCP(`mcp__unreal`)로
  실행 중인 에디터에 접속해 수행했다.

## 모델 프로비저닝

* 이 개발 PC에 `python`이 Windows Store 앱 실행 별칭(stub)만 있고 실제 Python이 없어
  `Scripts/DownloadKoreanVoiceModel.py`가 바로 실행되지 않았다.
  UE 5.8이 내장한 `Engine/Binaries/ThirdParty/Python3/Win64/python.exe`로 우회 실행해
  `VoiceModels/sherpa-onnx-streaming-zipformer-korean-2024-06-16/`(약 141MB)를 정상적으로 받았다.
  스크립트 자체는 수정하지 않았다.

# 변경 파일

신규
```text
Source/SuwonSiegeContestVR/Public/Core/Debug/VoiceKeywordTestWidget.h
Source/SuwonSiegeContestVR/Private/Core/Debug/VoiceKeywordTestWidget.cpp
Source/SuwonSiegeContestVR/Public/Core/Debug/VoiceKeywordTestActor.h
Source/SuwonSiegeContestVR/Private/Core/Debug/VoiceKeywordTestActor.cpp
Content/Maps/Debug/L_VoiceKeywordTest.umap
docs/Core/plans/2026-08-27_VOICE_KEYWORD_TEST_LEVEL.md
docs/Core/completed/2026-08-27_VOICE_KEYWORD_TEST_LEVEL.md
```

미커밋 (기존 규칙과 동일)
```text
VoiceModels/sherpa-onnx-streaming-zipformer-korean-2024-06-16/  (.gitignore 대상, 각자 다운로드)
```

# 주요 결정 사항

| 결정 | 이유 |
|---|---|
| 새 GameMode를 만들지 않고 기존 `ADebugFreeCameraGameMode` 재사용 | 이미 Core에 있는 비VR 테스트용 GameMode와 요구사항이 동일함. 중복 구현 금지 원칙 |
| `UVoiceRecognitionComponent`/`USherpaVoiceRecognitionComponent`를 수정하지 않고 상위 액터에서 재리스닝 루프만 추가 | 발화 1건 = 요청 1건 종료라는 기존 설계를 그대로 유지하면서, 연속 리스닝은 이 테스트 도구만의 요구사항이므로 호출부에서 해결 |
| 위젯을 `RebuildWidget()` 네이티브 코드로 구성 | 기존 `InitialConsonantQuizWidget`/`SubtitleWidget`과 동일한 패턴. WBP 디자이너 작업이 필요 없어 에디터 재시작 1회만으로 완결 |
| `L_XRTemplate` 복제로 레벨 생성 | 언리얼 MCP 툴셋에 "새 레벨 생성" 기능이 없어, 바닥+조명만 있는 기존 최소 템플릿을 복제하는 방식을 택함 |

# 테스트 결과

* Win64 **Game**(비에디터) 타깃 빌드 성공 (에디터는 실행 중이라 에디터 타깃은 DLL이 잠겨 있었음).
  최초 빌드에서 `UVerticalBoxSlot* Slot` 지역 변수가 `UWidget::Slot` 멤버를 가리는 C4458 오류가
  나 `StatusSlot`/`KeywordSlot` 등으로 이름을 바꿔 해결했다.
* 사용자가 에디터를 재시작해 Live Coding 없이 새 클래스를 로드했고, MCP `search_subclasses`로
  `AVoiceKeywordTestActor`가 인식됨을 확인했다.
* PIE 1차 실행(모델 다운로드 전): `ModelMissing` 상태로 크래시 없이 정상 폴백 — 위젯에
  "사용 불가 — ... ModelMissing ..." 문구가 정확히 표시됨을 스크린샷으로 확인.
* 모델 다운로드 후 PIE 2차 실행: 로그에서
  `Korean speech recognizer loaded in 1.30s` → `Capture device: 마이크 배열(Realtek(R) Audio)` →
  `Listening for 2 keyword(s) at 16000 Hz` 확인. 위젯 상태가 초록색 "듣는 중"으로 표시됨.
* **사람이 실제로 마이크에 대고 확인**: "옹성"은 반복 발화 중 다수 정확히 인식됨
  (`Recognized "옹성"`이 로그에 여러 번 찍힘). 위젯의 "감지된 키워드"·"최근 기록"도 정상 갱신됨.
* **"신기전"은 잘 인식되지 않음 — 버그 아님, 모델 정확도 한계로 확인**.
  같은 세션의 `LogSherpaVoice` 인식 결과 22건("영", "엉성", "공성", "동성", "홍성", "농담",
  "엉뚱한" 등)에 "신기전"과 비슷한 문자열이 한 건도 없었다. `UHangulTextLibrary::NormalizeAnswer`는
  공백/구두점 제거 + 소문자화만 하므로 2음절 "옹성"과 3음절 "신기전"을 다르게 처리할 여지가 없어
  매칭 로직 버그는 배제했다. 현재 `greedy_search` + hotwords 꺼짐 상태의 디코딩 정확도가
  원인으로 보인다.

# 남은 문제

* **"신기전" 인식률 개선은 별도 작업으로 분리**. `docs/Core/specs/SHERPA_ONNX_INTEGRATION.md` 8절에
  이미 기록된 hotwords 미사용 문제와 동일한 근본 원인이다. 개선하려면 sherpa-onnx의 `text2token`
  도구로 `bpe.vocab`을 만들고 `bUseHotwords=true` + `DecodingMethod=modified_beam_search`로
  전환해야 한다. 사용자 요청 시 진행하기로 하고 이번 작업 범위에서는 보류했다.
* Android 스탠드얼론에서의 이 테스트 레벨 검증은 범위 밖 (기존 `docs/Core/specs/SHERPA_ONNX_INTEGRATION.md`
  6절의 Android 미검증 상태가 그대로 적용됨).
