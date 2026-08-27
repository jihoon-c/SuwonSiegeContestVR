# 완료 기록 — 실제 음성 인식 백엔드(sherpa-onnx) 임포트 및 연동

**완료일**: 2026-08-27
**계층**: Core (ThirdParty 포함)
**선행 작업**: `docs/Core/completed/2026-08-27_INITIAL_CONSONANT_QUIZ.md`
**사양**: `docs/Core/specs/SHERPA_ONNX_INTEGRATION.md`

---

# 작업

초성 퀴즈의 음성 인식을 Mock에서 **실제 온디바이스 백엔드**로 교체했다.
PC(에디터·PC 빌드·Link 모드)에서 실제 한국어 음성을 인식하는 것을 실측 검증했고,
Android 스탠드얼론용 라이브러리·권한·모델 스테이징 경로까지 구성했다.

# 구현 내용

## ThirdParty 모듈

`Source/ThirdParty/SherpaOnnx/` — sherpa-onnx **v1.13.6** C API (Apache-2.0)

| 항목 | 내용 |
|---|---|
| Win64 | `sherpa-onnx-c-api.dll/.lib`, `onnxruntime.dll`, `onnxruntime_providers_shared.dll` (약 20MB) |
| Android arm64-v8a | `libsherpa-onnx-c-api.so`, `libonnxruntime.so` (약 26MB) |
| 헤더 | `include/sherpa-onnx/c-api/c-api.h` |
| `SherpaOnnx.Build.cs` | Win64는 delay-load + `RuntimeDependencies`, Android는 `PublicAdditionalLibraries` + APL |
| `SherpaOnnx_APL.xml` | APK에 `.so` 포함, 시작 시 로드, `RECORD_AUDIO` 매니페스트 추가 |

바이너리는 Git LFS로 커밋한다(`.gitattributes`에 `*.dll`, `*.lib` 추가).

## Core / Voice

* `USherpaVoiceRecognitionComponent` — `UVoiceRecognitionComponent` 구현체
  * `BeginPlay`에서 모델을 **백그라운드 로드**(PC 1.6초), 상태는 게임 스레드로 마샬링
  * UE `AudioCapture`로 마이크 입력 → mono 변환 → sherpa가 내부에서 16kHz 리샘플
  * 전용 디코드 워커 스레드: `AcceptWaveform → Decode → 부분 결과 검사`
  * **정답 키워드가 발화 안에서 보이면 즉시 보고**(무음 대기 없이). 아니면 endpoint에서 최종 결과
  * `DecodeWaveFile()` — 마이크 없이 백엔드를 검증하는 디버그·테스트 경로
  * `GetBackendDescription()` 오버라이드로 `ssv.voice.status`에 모델 상태·경로·오류 노출
  * Android에서 `RECORD_AUDIO` 런타임 권한 요청(`AndroidPermission` 플러그인)
  * 사용한 마이크 이름을 로그로 남긴다(Link 모드에서 장치 확인용)
* `UVoiceModelLibrary` — 모델 경로 해석
  * PC: `<Project>/VoiceModels/<model>/` 직접 사용
  * Android: OBB 안의 파일을 네이티브가 열 수 없으므로 **최초 1회 기기 저장소로 추출**
* `UVoiceRecognitionComponent`
  * 백엔드 시작 실패 시 **결과를 브로드캐스트하지 않도록 수정**
    (실패를 오답으로 소비해 시도가 즉시 소진되는 문제)
  * 콘솔 명령을 백엔드 무관하게 일반화: `ssv.voice.submit <텍스트>`, `ssv.voice.status`

## Core / Quiz

* `FallbackVoiceRecognitionClass` 도입 — 기본값이 **sherpa 백엔드**
  (`bSpawnMockVoiceRecognitionIfMissing` 대체). Mock은 선택지로 남았다
* `bPreloadVoiceRecognitionOnBeginPlay` — 레벨 로드 때 모델을 미리 올린다
* 인식기를 쓸 수 없는 시도에는 **제한 시간 타이머**를 걸어 흐름이 멈추지 않게 했다
* GameInstance가 없는 월드(자동화)에서는 인식기를 만들지 않는다

## 모델 제공

* `Scripts/DownloadKoreanVoiceModel.py` — `sherpa-onnx-streaming-zipformer-korean-2024-06-16`
  (encoder int8 127MB + decoder 11MB + joiner int8 2.6MB + tokens, 약 141MB)
* `.gitignore`에 `/VoiceModels/` — **모델은 커밋하지 않는다**
* `Config/DefaultGame.ini`에 `+DirectoriesToAlwaysStageAsNonUFS=(Path="VoiceModels")`

# 변경 파일

신규
```text
Source/ThirdParty/SherpaOnnx/SherpaOnnx.Build.cs
Source/ThirdParty/SherpaOnnx/SherpaOnnx_APL.xml
Source/ThirdParty/SherpaOnnx/include/sherpa-onnx/c-api/c-api.h
Source/ThirdParty/SherpaOnnx/Win64/*.dll|.lib
Source/ThirdParty/SherpaOnnx/Android/arm64-v8a/*.so
Source/SuwonSiegeContestVR/Public|Private/Core/Voice/SherpaVoiceRecognitionComponent.h|.cpp
Source/SuwonSiegeContestVR/Public|Private/Core/Voice/VoiceModelLibrary.h|.cpp
Source/SuwonSiegeContestVR/Private/Tests/SherpaVoiceRecognitionTests.cpp
Scripts/DownloadKoreanVoiceModel.py
docs/Core/specs/SHERPA_ONNX_INTEGRATION.md
```

수정
```text
Source/SuwonSiegeContestVR/SuwonSiegeContestVR.Build.cs   (AudioCapture / SherpaOnnx / AndroidPermission)
Source/SuwonSiegeContestVR/*/Core/Scenario/ScenarioInteractableComponent.cpp (비에디터 빌드 복구)
Source/SuwonSiegeContestVR/*/Core/Voice/VoiceRecognitionComponent.h|.cpp
Source/SuwonSiegeContestVR/*/Core/Voice/MockVoiceRecognitionComponent.h|.cpp
Source/SuwonSiegeContestVR/*/Core/Quiz/InitialConsonantQuizComponent.h|.cpp
SuwonSiegeContestVR.uproject      (NNERuntimeORT / NNEDenoiser 비활성화)
Config/DefaultGame.ini            (VoiceModels NonUFS 스테이징)
.gitattributes / .gitignore
docs/ARCHITECTURE.md / docs/COLLABORATION.md / docs/Core/specs/*
```

# 주요 결정 사항

| 결정 | 이유 |
|---|---|
| **엔진 `NNERuntimeORT` 플러그인 비활성화** | 엔진이 자체 `onnxruntime.dll`을 로드하면 sherpa가 그것에 바인딩되어 **모델 로드 중 크래시**한다(실측 확인). 이 프로젝트는 NNE를 쓰지 않는다 |
| 모델을 저장소에 커밋하지 않음 | 141MB. LFS라도 클론 비용이 크고, 스크립트 한 줄로 받을 수 있다 |
| 라이브러리는 커밋 | 없으면 빌드가 안 된다. 약 46MB, LFS 관리 |
| 기본 디코딩 `greedy_search`, hotwords 꺼짐 | hotwords는 `bpe.vocab`이 추가로 필요하다. 없는 파일을 전제로 기본값을 잡으면 초기화가 실패한다 |
| 발화 **안**의 키워드를 정답으로 인정 | 실제 발화는 "옹성이요"처럼 나온다. 인식기 단계에서 키워드를 뽑아 주고, 퀴즈 판정은 완전 일치를 유지 |
| 인식 결과를 부분 결과에서 조기 확정 | endpoint(무음 1.6초)를 기다리면 응답이 굼뜨다 |
| 모델 로드를 레벨 로드 시점으로 당김 | 퀴즈 시작 시 1.6초 정지를 피한다 |

# 테스트 결과

**Editor 타깃 빌드 성공. Win64 Game(비에디터) 타깃 빌드 성공. Automation 33건 전부 통과 (Exit Code 0).**

새 테스트 `Suwon.Core.Voice.SherpaKoreanDecode`는 **실제 모델로 실제 음성 파일을 인식**한다.

```text
Recognizer ready in 1.6s.
test_wavs/0.wav -> "걔는괜찮은척하려구애쓰는거같았다" (0.20s)   기준: 그는 괜찮은 척하려고 애쓰는 것 같았다.
test_wavs/1.wav -> "지하철에서다리를벌리고하진마라." (0.17s)   기준: 지하철에서 다리를 벌리고 앉지 마라.
```

* 모델 로드 1.6초, 3~4초 발화 디코딩 0.2초(RTF ≈ 0.06)
* 모델이 없으면 경고를 남기고 skip 처리되어 CI가 깨지지 않는다
* 기존 32건 회귀 없음

# 남은 문제

1. **Android 빌드·기기 검증 미실시.** 개발 PC에 Android NDK와 cmdline-tools가 없어
   패키징을 실행하지 못했다. 설치 후 `docs/Core/specs/SHERPA_ONNX_INTEGRATION.md` 6절 절차를 따른다.
   코드·APL·권한·모델 스테이징은 모두 구성되어 있다.
2. **실제 마이크 발화 검증은 사람이 해야 한다.** 이 환경에는 마이크가 없어
   음성 파일 디코딩까지만 검증했다. PC 절차는 같은 문서 5절 참조.
3. hotwords(정답 부스팅) 미사용 — 인식률이 부족하면 `bpe.vocab` 생성 후 켠다.
4. 잡음 환경·초등 화자 인식률 미측정.
5. ~~Game(비에디터) 타깃 빌드 실패~~ → **이번 작업에서 해결했다.**
   `Core/Scenario/ScenarioInteractableComponent.cpp`가 `WITH_EDITORONLY_DATA` 전용
   `IsVisualizationComponent()`를 무조건 호출하던 것을 가드했다. Android 패키징의 선행 조건이라
   범위에 포함했다. **Win64 Game 타깃 빌드 성공을 확인했다.**
