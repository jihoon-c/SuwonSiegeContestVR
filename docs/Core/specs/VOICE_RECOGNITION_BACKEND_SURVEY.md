# 온디바이스 한국어 음성 인식 / 키워드 감지 백엔드 조사

**조사일**: 2026-08-27
**대상**: Android 스탠드얼론 VR (arm64-v8a, Quest 계열)
**용도**: 초성 퀴즈 정답 발화 인식 — 어휘가 4~10개로 제한된 **키워드 감지** 문제
**관련 결정**: `docs/ARCHITECTURE.md` — "음성 인식은 온디바이스에서 동작, 외부 서드파티 모듈 임포트"

> **2026-08-27 갱신: 1순위 후보인 sherpa-onnx를 실제로 임포트해 연동을 마쳤다.**
> 연동 내용·검증 결과·주의사항은 `docs/Core/specs/SHERPA_ONNX_INTEGRATION.md`를 참조한다.
> 아래는 선정 근거로 남겨둔 조사 기록이다.

---

## 1. 결론 요약

| 순위 | 후보 | 근거 |
|---|---|---|
| **1** | **sherpa-onnx + 한국어 Zipformer Transducer (스트리밍) + hotwords** | 사용자가 VR 스탠드얼론에서 동작을 확인한 계열. Apache-2.0. int8 약 132MB. 어휘 제한 대신 **hotwords(contextual biasing)** 로 정답 단어를 부스팅 |
| 2 | Vosk (`vosk-model-small-ko-0.22`) | 49.7MB로 가장 가볍고 Android AAR 제공. **문법(grammar) 제한**을 지원해 소어휘 정확도가 매우 높음. Apache-2.0 |
| 3 | sherpa-onnx KWS (`KeywordSpotter`) 를 한국어로 재학습 | 3M 파라미터로 가장 가볍지만 **한국어 사전학습 모델이 없다.** icefall + KsponSpeech 학습 필요 |
| 4 | Picovoice Porcupine / Rhino | 매우 가볍고 정확하나 **상용 라이선스**. 교육 콘텐츠 배포 조건 확인 필요 |

**결정 (2026-08-27)**: sherpa-onnx v1.13.6 + 한국어 스트리밍 Zipformer를 채택해 연동을 완료했다.
PC에서 모델 로드 1.6초, 발화 디코딩 RTF 0.06을 실측했다.
용량(141MB)이 문제가 되면 Vosk small ko(49.7MB) + grammar로 교체할 수 있고,
그때도 `UVoiceRecognitionComponent` 하위 클래스만 바꾸면 되므로 게임플레이 코드는 영향받지 않는다.

---

## 2. sherpa-onnx

k2-fsa/next-gen Kaldi의 onnxruntime 런타임. 인터넷 없이 동작하며
Android/iOS/임베디드/RISC-V를 공식 지원한다. 라이선스 Apache-2.0.

### 2.1 키워드 감지(KWS) 전용 모델 — 한국어 없음

공식 사전학습 KWS 모델은 3종뿐이다.

| 모델 | 언어 | 파라미터 | 패키지 크기 |
|---|---|---|---|
| `sherpa-onnx-kws-zipformer-zh-en-3M-2025-12-20` | 중국어·영어 | 3M | 38MB |
| `sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01` | 중국어 | 3.3M | 18MB |
| `sherpa-onnx-kws-zipformer-gigaspeech-3.3M-2024-01-01` | 영어 | 3.3M | 19MB |

`KeywordSpotter`는 **open vocabulary** 방식이라 재학습 없이 키워드를 바꿀 수 있지만,
이는 **해당 언어 토큰 범위 안에서만** 성립한다. 한국어 모델이 없으므로
`옹성`·`공심돈` 같은 단어를 그대로 넣을 수 없다.
한국어 KWS를 쓰려면 icefall로 KsponSpeech 기반 KWS 학습이 필요하다(별도 R&D 범위).

### 2.2 한국어 ASR 모델 (실제 사용 가능한 경로)

| 모델 | 형태 | 크기 (int8) | 비고 |
|---|---|---|---|
| `sherpa-onnx-streaming-zipformer-korean-2024-06-16` | 스트리밍 Transducer | encoder 127MB + decoder 2.84MB + joiner 2.58MB ≈ **132MB** | KsponSpeech 학습. fp32는 encoder 293MB |
| `sherpa-onnx-zipformer-korean-2024-06-24` | 오프라인(발화 단위) Transducer | 유사 | 발화 종료 후 일괄 인식 |

**스트리밍 + hotwords** 조합이 이 프로젝트에 맞는다.

* Transducer 모델만 hotwords를 지원하고, 디코딩을 `modified_beam_search`로 바꿔야 한다
  (기본 `greedy_search`는 미지원).
* hotwords는 Aho-Corasick 오토마톤으로 구현되어 있고, 부스팅 점수를 올리면
  해당 단어가 beam search에서 살아남기 쉬워진다.
* 초성 퀴즈는 정답 어휘가 이미 정해져 있으므로 퀴즈 시작 시 정답·별칭을 hotwords로 주입하면 된다.

### 2.3 UE5 / Android 통합 경로

* sherpa-onnx는 **C API**(`libsherpa-onnx-c-api.so`)와 Android arm64-v8a 프리빌트 라이브러리를 제공한다.
* 공식 Unreal 플러그인은 없다. Unity용 커뮤니티 플러그인은 존재하며,
  동일한 방식(네이티브 .so + 바인딩)을 UE ThirdParty 모듈로 옮기면 된다.
* UE 통합 형태(예정):
  1. `Source/ThirdParty/SherpaOnnx/` 에 헤더 + arm64-v8a/Win64 라이브러리 배치
  2. `Build.cs`에서 `PublicAdditionalLibraries` / `PublicDelayLoadDLLs` 연결,
     Android는 `AdditionalPropertiesForReceipt`로 APL XML 등록
  3. 모델 파일은 `Content/NonUFS` 또는 OBB 경로에 배치하고 런타임에 절대 경로로 로드
  4. 마이크 입력은 UE `IAudioCaptureFactory` / `AudioCapture` 모듈로 받아 16kHz mono float로 변환
  5. `USherpaVoiceRecognitionComponent : UVoiceRecognitionComponent` 로 감싸고
     인식 결과를 `ReportRecognizedText()`에 넣는다

게임플레이 쪽 변경은 **컴포넌트 클래스 교체 한 줄**이다.

---

## 3. Vosk

Kaldi 기반 오프라인 인식기. Android AAR과 `vosk-model-small-ko-0.22`(**49.7MB**)를 제공하며
라이선스는 Apache-2.0이다.

**장점**
* 용량이 sherpa 한국어 모델의 약 1/3
* **grammar 제한**을 지원한다. 인식 어휘를 `["옹성","공심돈","녹로","거중기","[unk]"]`로
  못 박으면 소어휘 정확도가 크게 오른다. 초성 퀴즈에 이상적이다.
* Android 통합 사례가 매우 많다

**단점**
* JNI 경유(Java AAR)라 UE Android 통합 시 JNI 브리지 코드가 필요하다.
  (UE는 `FJavaWrapper`/JNI 호출을 지원하므로 불가능하지는 않다)
* 데스크톱 에디터 테스트용 Win64 바이너리를 별도로 챙겨야 한다

---

## 4. 기타 후보

| 후보 | 평가 |
|---|---|
| **openWakeWord / microWakeWord** | 초경량 웨이크워드 전용. 한국어 단어는 TTS 합성 데이터로 직접 학습해야 하고, 퀴즈마다 모델을 늘려야 해 확장성이 나쁘다 |
| **Picovoice Porcupine** | 한국어 지원·초경량·정확. 다만 상용 라이선스이며 액세스 키 기반이라 오프라인 배포 조건 확인 필요 |
| **SenseVoice-Small (sherpa-onnx 지원)** | 한국어 포함 다국어 오프라인 모델이나 int8도 200MB 이상이라 VR 스탠드얼론에는 과하다 |
| **Android SpeechRecognizer** | 기기·언어팩 의존이고 오프라인 보장이 없어 프로젝트 결정(온디바이스 보장)과 충돌 |

---

## 5. 선정 기준 체크리스트

백엔드를 확정할 때 다음을 실측한다.

- [ ] arm64-v8a 실기(Quest) 동작 및 **인식 지연 500ms 이하**
- [ ] 정답 4단어(옹성·공심돈·녹로·거중기) 인식률, 특히 **초등~중등 화자**
- [ ] 오탐(false accept) 비율 — 교관 나레이션·전투 효과음이 겹칠 때
- [ ] APK/OBB 증가량과 런타임 메모리
- [ ] 라이선스(Apache-2.0 선호) 및 배포 조건
- [ ] 마이크 권한 요청 UX (Android `RECORD_AUDIO`)

---

## 6. 출처

* [sherpa-onnx (GitHub)](https://github.com/k2-fsa/sherpa-onnx)
* [Keyword spotting — sherpa 문서](https://k2-fsa.github.io/sherpa/onnx/kws/index.html)
* [KWS 사전학습 모델 목록](https://k2-fsa.github.io/sherpa/onnx/kws/pretrained_models/index.html)
* [Hotwords (Contextual biasing) — sherpa 문서](https://k2-fsa.github.io/sherpa/onnx/hotwords/index.html)
* [sherpa-onnx-streaming-zipformer-korean-2024-06-16 (Hugging Face)](https://huggingface.co/k2-fsa/sherpa-onnx-streaming-zipformer-korean-2024-06-16)
* [sherpa-onnx-zipformer-korean-2024-06-24 (Hugging Face)](https://huggingface.co/k2-fsa/sherpa-onnx-zipformer-korean-2024-06-24)
* [Unity integration plugin for sherpa-onnx (Discussion #3198)](https://github.com/k2-fsa/sherpa-onnx/discussions/3198)
* [VOSK Models](https://alphacephei.com/vosk/models)
* [Offline speech recognition on Android with VOSK](https://alphacephei.com/vosk/android)
