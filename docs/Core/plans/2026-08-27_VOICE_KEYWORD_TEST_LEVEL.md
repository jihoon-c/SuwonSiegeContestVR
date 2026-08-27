# 키워드 감지 마이크 테스트 레벨

**계층**: Core (Debug)
**작성일**: 2026-08-27
**상태**: `Done` — 완료 기록 `docs/Core/completed/2026-08-27_VOICE_KEYWORD_TEST_LEVEL.md` 참조
**선행 문서**: `docs/Core/specs/SHERPA_ONNX_INTEGRATION.md`, `docs/Core/completed/2026-08-27_SHERPA_ONNX_BACKEND.md`

---

# 목적

노트북 기본 마이크로 실제 발화를 입력받아 sherpa-onnx 백엔드가 인식하는 문장과,
그 안에서 "옹성"·"신기전" 키워드가 감지되는지를 퀴즈/시나리오와 무관하게 화면 위젯으로
바로 확인할 수 있는 독립 테스트 레벨을 만든다.

# 현재 상태

* 실제 음성 인식 백엔드(`USherpaVoiceRecognitionComponent`)는 이미 연동되어 있고
  `LV_Ongseong`의 초성 퀴즈에서 검증되었다 (`docs/Core/completed/2026-08-27_SHERPA_ONNX_BACKEND.md`).
* `UVoiceRecognitionComponent`의 요청은 **발화 1건마다 종료**되는 단발 구조라
  (매 `ReportRecognizedText` 호출마다 `FinishRequest` → Idle), 연속 리스닝을 보여주는
  전용 도구는 없다.
* 마이크 없이 텍스트를 흘려보내는 `ssv.voice.submit <text>` 콘솔 명령은 리스닝 중인
  아무 레코그나이저에나 전달되므로, 이번에 추가하는 레코그나이저에도 그대로 동작한다.
* Core에는 비VR 데스크톱 테스트용 `ADebugFreeCameraGameMode`/`ADebugFreeCameraPawn`이
  이미 있다 (`docs/OngseongCrossbow/plans/2026-08-24_NON_VR_COMBAT_TEST_GAMEMODE.md`).

# 구현 범위

## Core (`Source/SuwonSiegeContestVR/*/Core/Debug/`)

* `UVoiceKeywordTestWidget : UUserWidget` — 상태 / 마지막 인식 문장 / 감지된 키워드 /
  최근 기록을 보여주는 네이티브 HUD 위젯. `RebuildWidget()`으로 트리를 직접 구성해
  WBP 디자이너 작업 없이 동작한다 (`UInitialConsonantQuizWidget` 패턴 재사용).
* `AVoiceKeywordTestActor : AActor` — `USherpaVoiceRecognitionComponent`를 소유하고
  * `BeginPlay`에서 위젯을 뷰포트에 추가
  * 모델이 준비되는 대로 `Keywords=["옹성","신기전"]`, `ListenDuration=0`으로 리스닝 시작
  * `OnRecognitionResult`를 받을 때마다 위젯을 갱신하고 **즉시 재리스닝**해 끊김 없이 반복
  * `OnVoiceStateChanged`로 모델 로딩/미존재/오류 상태를 위젯에 노출

기존 `UVoiceRecognitionComponent`/`USherpaVoiceRecognitionComponent` API는 수정하지 않는다.

## 레벨

* `Content/Maps/Debug/L_VoiceKeywordTest.umap` — `L_XRTemplate`(바닥+조명만 있는 템플릿)을
  복제해 만든 최소 레벨. World Settings의 GameMode Override를 `ADebugFreeCameraGameMode`로
  지정해 VR HMD/Pawn 없이 PIE로 바로 진입한다.
* `AVoiceKeywordTestActor` 인스턴스 1개를 배치한다.
* 언리얼 MCP(`mcp__unreal`)로 에디터에 접속해 레벨 복제·GameMode 지정·액터 배치·저장을 수행한다.
  MCP 툴셋에는 "새 레벨 생성" 기능이 없어 기존 최소 템플릿 레벨을 복제하는 방식을 쓴다.

# 변경 예정 파일

신규
```text
Source/SuwonSiegeContestVR/Public/Core/Debug/VoiceKeywordTestWidget.h
Source/SuwonSiegeContestVR/Private/Core/Debug/VoiceKeywordTestWidget.cpp
Source/SuwonSiegeContestVR/Public/Core/Debug/VoiceKeywordTestActor.h
Source/SuwonSiegeContestVR/Private/Core/Debug/VoiceKeywordTestActor.cpp
Content/Maps/Debug/L_VoiceKeywordTest.umap
```

# 구현 단계

1. `UVoiceKeywordTestWidget`/`AVoiceKeywordTestActor` C++ 작성.
2. Win64 Game 타깃 빌드로 컴파일 검증 (에디터 타깃은 실행 중인 에디터가 DLL을 잠그고 있어
   에디터 안에서 Live Coding/Compile로 반영해야 한다).
3. 에디터에 새 클래스가 반영되면 MCP로 레벨 복제 → GameMode Override → 액터 배치 → 저장.
4. PIE로 열어 모델 로딩 상태와 위젯 표시를 확인한다. 실제 발화 인식은 마이크가 있는
   사람이 직접 말해서 확인해야 한다 (`ssv.voice.submit <텍스트>`로 마이크 없이도 흐름 확인 가능).

# 다른 Feature에 미치는 영향

없음. Core에 신규 클래스만 추가하며 기존 Quiz/Voice 코드나 GF_OngseongCrossbow의 실제
퀴즈 플로우는 건드리지 않는다.

# 검증 방법

* Win64 Game 타깃 컴파일 성공
* PIE에서 `ssv.voice.status`로 레코그나이저가 `Ready` 상태인지 확인
* "옹성", "신기전" 발화 시 위젯에 인식 문장과 감지 키워드가 표시되는지 확인
* 관련 없는 문장을 말했을 때도 인식 문장은 표시되고 키워드 칸은 비어 있는지 확인
