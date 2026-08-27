# sherpa-onnx 온디바이스 한국어 음성 인식 연동

**계층**: Core
**작성일**: 2026-08-27
**상태**: `Status: Implemented (PC 검증 완료 / Android 빌드 검증 대기)`
**관련 문서**: `docs/Core/specs/INITIAL_CONSONANT_QUIZ.md`,
`docs/Core/specs/VOICE_RECOGNITION_BACKEND_SURVEY.md`

---

## 1. 무엇을 임포트했는가

| 항목 | 내용 |
|---|---|
| 런타임 | **sherpa-onnx v1.13.6** C API (Apache-2.0) |
| Windows | `Source/ThirdParty/SherpaOnnx/Win64/` — `sherpa-onnx-c-api.dll/.lib`, `onnxruntime.dll`, `onnxruntime_providers_shared.dll` (약 20MB) |
| Android | `Source/ThirdParty/SherpaOnnx/Android/arm64-v8a/` — `libsherpa-onnx-c-api.so`, `libonnxruntime.so` (약 26MB) |
| 헤더 | `Source/ThirdParty/SherpaOnnx/include/sherpa-onnx/c-api/c-api.h` |
| 모델 | **sherpa-onnx-streaming-zipformer-korean-2024-06-16** (KsponSpeech 학습, 스트리밍 Transducer) |
| 모델 크기 | encoder int8 127MB + decoder 11MB + joiner int8 2.6MB + tokens 60KB ≈ **141MB** |

바이너리는 Git LFS로 커밋한다(`.gitattributes`에 `*.dll`, `*.lib`, `*.so`).
**모델은 커밋하지 않는다.** `Scripts/DownloadKoreanVoiceModel.py`로 각자 내려받는다.

---

## 2. 반드시 알아야 할 제약 — NNE 플러그인 비활성화

**언리얼 엔진의 `NNERuntimeORT` 플러그인은 자체 `onnxruntime.dll`을 로드한다.**
Windows는 DLL 임포트를 **파일 이름**으로 해석하므로, 엔진 ORT가 먼저 로드되면
`sherpa-onnx-c-api.dll`이 요구하는 ORT 대신 엔진 것에 바인딩된다.
sherpa는 ORT 1.27 API를, 엔진은 그보다 낮은 버전을 쓰기 때문에
`OrtGetApiBase()->GetApi(...)`가 null을 반환하고 **모델 로드 중 즉시 크래시**한다
(실제로 `EXCEPTION_ACCESS_VIOLATION reading 0x18`로 재현했다).

따라서 `SuwonSiegeContestVR.uproject`에서 다음을 비활성화했다.

```json
{ "Name": "NNERuntimeORT", "Enabled": false },
{ "Name": "NNEDenoiser",   "Enabled": false }
```

이 프로젝트는 NNE 기능을 사용하지 않는다.
**누군가 NNE를 다시 켜면 음성 인식이 크래시한다.** 그때는 sherpa를 static link 빌드로
교체해야 하며, 이는 별도 작업이다.

---

## 3. 런타임 구조

```text
UInitialConsonantQuizComponent
        │  StartListening(Keywords, ListenDuration)
        ▼
USherpaVoiceRecognitionComponent  (Core/Voice)
        ├─ UE AudioCapture ──▶ 마이크 float PCM (기기 샘플레이트 그대로)
        │                       sherpa가 내부에서 16kHz로 리샘플
        ├─ 디코드 워커 스레드 ──▶ AcceptWaveform → Decode → 부분 결과
        │                       정답 키워드가 보이면 즉시 보고(끝날 때까지 기다리지 않음)
        │                       아니면 endpoint(무음 1.6초)에서 최종 결과 보고
        └─ ReportRecognizedText() ──▶ 게임 스레드로 마샬링 → 퀴즈 판정
```

주요 설계 결정

| 결정 | 이유 |
|---|---|
| 모델은 `BeginPlay`에서 **비동기 로드** | 로드에 1~2초가 걸린다. 퀴즈 시작 시점에 로드하면 눈에 띈다 |
| 퀴즈 컴포넌트가 레벨 로드 시 인식기를 **미리 확보** | 위와 같은 이유. `bPreloadVoiceRecognitionOnBeginPlay` |
| 발화 **안에서** 키워드를 찾으면 정답 인정 | 플레이어는 "옹성이요"처럼 말한다. 퀴즈 자체의 판정은 여전히 완전 일치 |
| 디코딩은 기본 `modified_beam_search`, hotwords 기본 켜짐 | 9절 참조. `bpe.vocab`이 없으면 경고 후 `greedy_search`로 자동 폴백하므로, 스크립트를 다시 돌리지 않은 체크아웃도 그대로 동작한다 |
| 마이크 장치는 요청이 끝나도 `CaptureIdleTimeout`(기본 2초) 동안 열어 둔다 | 연속 리스닝 호출부가 같은 프레임에 재요청하는데, 그때 장치를 닫았다 여는 비용이 다음 발화 앞음절을 삼킨다. 유휴 구간에는 콜백이 샘플을 버리므로 녹음되지 않는다 |
| 인식기 시작 실패 시 결과를 브로드캐스트하지 않음 | 실패를 "오답"으로 소비하면 시도가 즉시 소진된다 |
| GameInstance가 없는 월드(자동화)에서는 인식기를 만들지 않음 | 테스트가 141MB 모델을 로드할 이유가 없다 |

---

## 4. 모델 준비

```bash
python Scripts/DownloadKoreanVoiceModel.py
```

* 받는 위치: `<Project>/VoiceModels/sherpa-onnx-streaming-zipformer-korean-2024-06-16/`
* `.gitignore`에 등록되어 커밋되지 않는다
* **`bpe.vocab`은 다운로드가 아니라 스크립트가 `bpe.model`에서 생성한다.** 업스트림 저장소에
  `bpe.vocab`이 없기 때문이다. hotwords에 필요하므로 **이미 모델을 받아둔 사람도 스크립트를
  한 번 더 실행해야 한다**(다른 파일은 `[skip]` 되고 `bpe.vocab`만 새로 만들어진다).
  sentencepiece 패키지 없이 `bpe.model` protobuf를 직접 읽으므로 추가 설치는 필요 없다
* 패키징: `Config/DefaultGame.ini`의
  `+DirectoriesToAlwaysStageAsNonUFS=(Path="VoiceModels")` 가 패키지에 포함시킨다
* **Android**: 스테이징된 파일은 OBB 안에 들어가 네이티브 라이브러리가 경로로 열 수 없다.
  `UVoiceModelLibrary::ResolveNativeModelDirectory()`가 최초 1회
  `ProjectPersistentDownloadDir`로 복사한 뒤 그 경로를 sherpa에 넘긴다.

모델이 없으면 크래시하지 않는다. 인식기 상태가 `ModelMissing`이 되고,
퀴즈는 "음성 인식을 사용할 수 없습니다"를 띄운 뒤 제한 시간마다 시도를 소모하며 진행된다.

`bpe.vocab`만 없는 경우는 `ModelMissing`이 아니다. 필수 파일은 encoder/decoder/joiner/tokens
네 개(`GetRequiredModelFiles()`)뿐이고, `bpe.vocab`은 선택 파일(`GetOptionalModelFiles()`)이라
없으면 경고만 남기고 hotwords 없이 인식한다.

---

## 5. PC · PC 연결(Link) 모드 검증

PC 빌드와 Link 모드는 **같은 Win64 바이너리**다. 별도 설정이 없다.

1. `python Scripts/DownloadKoreanVoiceModel.py`
2. 에디터에서 `LV_Ongseong` PIE 실행
3. 콘솔에서 `ssv.voice.status` — 다음처럼 나와야 한다
   ```
   Voice recognizer on BP_OngseongDefenseScenarioManager:
     USherpaVoiceRecognitionComponent [Idle], model sherpa-onnx-...-korean-...: Ready | files: D:/.../VoiceModels/...
   ```
4. 총통 장전을 마치면 초성 퀴즈가 뜬다. **"옹성"이라고 말한다.**
5. 로그에서 다음을 확인한다
   ```
   LogSherpaVoice: Display: Capture device: <마이크 이름> (2 ch, 48000 Hz).
   LogSherpaVoice: Display: Listening for 1 keyword(s) at 48000 Hz.
   LogSherpaVoice: Display: Recognized "옹성".
   ```

**마이크 선택 주의**: Link 모드에서 헤드셋 마이크는 Windows 입력 장치 중 하나일 뿐이다.
UE는 **OS 기본 입력 장치**를 사용하므로, 헤드셋 마이크로 테스트하려면 Windows
사운드 설정에서 기본 입력 장치를 헤드셋으로 바꾼다. 실제로 어떤 장치가 열렸는지는
위의 `Capture device:` 로그로 확인한다.

마이크 없이 흐름만 확인하려면 콘솔에서 `ssv.voice.submit 옹성` 을 쓴다.

---

## 6. Android 스탠드얼론 검증

### 6.1 준비 (이 저장소 환경에는 아직 없음)

UE 5.8 Android 툴체인이 필요하다. 현재 개발 PC에는 **NDK와 cmdline-tools가 설치되어 있지 않아
Android 빌드를 실행하지 못했다.**

```bash
# Android Studio에서 SDK Tools > NDK (Side by side) + Command-line Tools 설치 후
"C:\Program Files\Epic Games\UE_5.8\Engine\Extras\Android\SetupAndroid.bat"
```

> **선행 조건 해결됨**: 이전에는 `ScenarioInteractableComponent.cpp`가 에디터 전용 API를 호출해
> 비에디터 타깃 자체가 빌드되지 않았다. 2026-08-27에 가드를 넣어 **Win64 Game 타깃 빌드 성공**을
> 확인했으므로 Android 컴파일을 막는 코드 문제는 남아 있지 않다.

### 6.2 패키징

```bash
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
  -project="D:\Github\SuwonSiegeContestVR\SuwonSiegeContestVR.uproject" ^
  -platform=Android -clientconfig=Development -cook -stage -package -build ^
  -archive -archivedirectory="D:\Build\Ongseong"
```

패키징 시 자동으로 처리되는 것

* `SherpaOnnx_APL.xml`이 `libsherpa-onnx-c-api.so`, `libonnxruntime.so`를 APK의
  `libs/arm64-v8a`에 넣고 시작 시 로드한다
* 같은 APL이 매니페스트에 `android.permission.RECORD_AUDIO`를 추가한다
* `USherpaVoiceRecognitionComponent`가 `BeginPlay`에서 런타임 권한을 요청한다
  (`AndroidPermission` 플러그인)
* `VoiceModels/`가 NonUFS로 스테이징되고, 최초 실행 시 기기 저장소로 추출된다

### 6.3 기기 확인

```bash
adb logcat -s UE:V | findstr /C:"LogSherpaVoice"
```

기대 로그

```text
LogSherpaVoice: Requesting the RECORD_AUDIO permission.
LogSherpaVoice: Extracting voice model file encoder-epoch-99-avg-1.int8.onnx to the device.
LogSherpaVoice: Korean speech recognizer loaded in X.XXs (...)
LogSherpaVoice: Capture device: ...
LogSherpaVoice: Recognized "옹성".
```

### 6.4 Android에서 확인해야 할 것

- [ ] 최초 실행 시 141MB 추출에 걸리는 시간과 저장 공간
- [ ] 모델 로드 시간(PC에서는 1.6초, Quest CPU에서는 더 걸린다)
- [ ] 전투 중 CPU 여유 — 인식은 퀴즈 구간에서만 돌지만 `NumThreads`(기본 2) 조정 필요 여부
- [ ] 헤드셋 마이크 입력 레벨과 실제 인식률
- [ ] APK/OBB 증가량

---

## 7. 검증 결과 (PC)

`Suwon.Core.Voice.SherpaKoreanDecode` 자동화 테스트가 **실제 모델로 실제 음성 파일을 인식**한다.
마이크가 없는 빌드 머신에서도 백엔드가 살아 있는지 확인할 수 있다.

```text
Recognizer ready in 1.6s.
test_wavs/0.wav -> "걔는괜찮은척하려구애쓰는거같았다" (0.20s)   기준: 그는 괜찮은 척하려고 애쓰는 것 같았다.
test_wavs/1.wav -> "지하철에서다리를벌리고하진마라." (0.17s)   기준: 지하철에서 다리를 벌리고 앉지 마라.
```

* 모델 로드 **1.6초**, 3~4초 발화 디코딩 **0.2초**(RTF ≈ 0.06) — VR 프레임에 부담이 없다
* 모델이 없으면 테스트는 경고를 남기고 통과(skip)한다
* **출력에 띄어쓰기가 없다.** 정답 판정은 공백을 제거하고 비교하므로 문제되지 않는다

---

## 8. 남은 문제

* **Android 빌드·기기 검증 미실시** (NDK 미설치). 6절 절차 필요
* **hotwords 전환 후 실제 마이크 인식률 미측정** — 9절 참조. PC에서 모델이 새 설정으로
  로드되고 test_wavs 디코딩이 유지되는 것까지만 확인했다
* 잡음 환경(전투 효과음·나레이션과 겹칠 때) 인식률 미측정.
  현재는 퀴즈 구간에 전투가 시작되지 않도록 시나리오로 막아 두었다
* 화자 연령대(초·중등) 인식률 미측정

---

## 9. Hotwords와 마이크 캡처 수명 (2026-08-27 변경)

키워드 테스트 레벨에서 **"옹성"은 인식되는데 "신기전"은 거의 인식되지 않는** 문제를 다룬 변경이다.
원인 조사에서 다음 두 가지는 원인이 **아님**이 확인되었다.

* **모델 버전**: `sherpa-onnx-streaming-zipformer-korean-2024-06-16`이 지금도 k2-fsa의 최신
  한국어 모델이다. 다른 하나는 오프라인판 `sherpa-onnx-zipformer-korean-2024-06-24`뿐이고
  2025년 이후 한국어 모델은 없다. 한국어 전용 KWS(`KeywordSpotter`) 사전학습 모델은 존재하지
  않으므로 "키워드 감지 모델 교체"라는 선택지 자체가 없다
* **VAD**: 이 프로젝트는 sherpa의 VAD API를 쓰지 않는다. 마이크 콜백이 받은 샘플은 임계값 없이
  전부 `AcceptWaveform`으로 들어간다. `enable_endpoint`는 무음으로 발화 끝을 판정할 뿐 오디오를
  버리지 않으며, 최종 결과 보고 경로가 여기에 걸려 있어 끄면 결과가 나오지 않는다

### 9.1 Hotwords

`ResolveSpokenKeyword()`는 정규화 후 **정확 문자열**을 비교한다. 3음절 "신기전"은 한 음절만
어긋나도("신기절", "심기전") 매칭이 실패하는 반면 2음절 "옹성"은 훨씬 쉽게 맞는다.
`greedy_search`의 디코딩 정확도가 그대로 매칭 실패로 이어진 것이다.

바뀐 기본값:

| 항목 | 이전 | 현재 |
|---|---|---|
| `DecodingMethod` | `greedy_search` | `modified_beam_search` |
| `bUseHotwords` | `false` | `true` |

`StartListening`에 넘긴 `Keywords`가 그대로 hotwords가 된다
(`SherpaOnnxCreateOnlineStreamWithHotwords`, 줄바꿈 구분 raw 텍스트).
sherpa는 이 raw 텍스트를 `modeling_unit="bpe"` + `bpe_vocab`으로 BPE 단위에 매핑한다.

**폴백**: `bpe.vocab`이 없으면 초기화를 실패시키지 않고 경고 후 hotwords를 끄며
`greedy_search`로 되돌린다. hotwords가 켜져 있는데 `DecodingMethod`가 다른 값이면
`modified_beam_search`로 강제한다. 실제로 무엇이 적용됐는지는 `ssv.voice.status` 출력의
`| modified_beam_search, hotwords on` 부분과 로드 로그에서 확인한다.

### 9.2 마이크 캡처 수명

`UVoiceRecognitionComponent`는 발화 1건 = 요청 1건으로 끝나는 단발 구조다. 연속 리스닝 호출부
(`AVoiceKeywordTestActor`)는 결과를 받은 **같은 프레임에** 다시 `StartListening`을 부르는데,
이전 구조에서는 그 사이에 `StopCapture()` → `StartCapture()`로 오디오 장치를 닫았다 다시 열었다.
잡음 오인식 1건마다 마이크가 재오픈되므로, 그 순간 말을 시작하면 앞음절이 통째로 사라진다.

이제 `EndBackendListening`은 장치를 바로 닫지 않고 `CaptureIdleTimeout`(기본 2초) 타이머를 건다.
그 안에 새 요청이 오면 타이머를 취소하고 열려 있는 장치를 그대로 재사용한다.

**유휴 구간에는 아무것도 녹음되지 않는다.** 캡처 콜백이 `bAcceptSamples`를 보고 샘플을 버리므로
버퍼가 자라지도, 디코더에 들어가지도 않는다. `CaptureIdleTimeout = 0`으로 두면 이전처럼
요청이 끝나는 즉시 장치를 닫는다.
