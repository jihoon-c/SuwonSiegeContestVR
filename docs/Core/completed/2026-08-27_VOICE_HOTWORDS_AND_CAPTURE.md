# 완료 기록 — 음성 키워드 인식: hotwords 전환과 마이크 캡처 수명

**완료일**: 2026-08-27
**계층**: Core (Voice)
**선행 작업**: `docs/Core/completed/2026-08-27_VOICE_KEYWORD_TEST_LEVEL.md`
**스펙**: `docs/Core/specs/SHERPA_ONNX_INTEGRATION.md` 9절

---

# 작업

키워드 테스트 레벨(`L_VoiceKeywordTest`)에서 **"옹성"은 인식되는데 "신기전"은 거의 인식되지 않는**
문제를 조사하고 고쳤다.

사용자가 지목한 두 가설을 먼저 확인했고, 둘 다 원인이 아니었다.

| 확인 항목 | 결과 |
|---|---|
| 한국어 모델이 최신인가 | **최신 맞음.** `sherpa-onnx-streaming-zipformer-korean-2024-06-16`이 지금도 k2-fsa의 최신 한국어 모델이다. 나머지 하나는 오프라인판 `-2024-06-24`뿐이고 2025년 이후 한국어 모델은 없다. 한국어 전용 KWS 사전학습 모델은 존재하지 않으므로 "키워드 감지 모델 교체" 선택지 자체가 없다 |
| VAD가 켜져 있는가 | **쓰고 있지 않음. 끌 대상이 없었다.** 프로젝트 코드에 VAD API 호출이 한 건도 없고, 마이크 콜백이 받은 샘플은 임계값 없이 전부 `AcceptWaveform`으로 들어간다. 가장 비슷한 `enable_endpoint`는 무음으로 발화 끝을 판정할 뿐 오디오를 버리지 않으며, 최종 결과 보고 경로가 여기 걸려 있어 끄면 결과가 나오지 않는다. 그래서 끄지 않았다 |

실제 원인으로 두 가지를 고쳤다.

# 구현 내용

## A. hotwords + modified_beam_search

`ResolveSpokenKeyword()`는 정규화 후 정확 문자열을 비교한다. 3음절 "신기전"은 한 음절만 어긋나도
매칭이 실패하는 반면 2음절 "옹성"은 훨씬 쉽게 맞는다. `greedy_search`의 디코딩 정확도가 그대로
매칭 실패로 이어졌다.

hotwords 배선(`modeling_unit="bpe"`, `bpe_vocab`, `CreateOnlineStreamWithHotwords`)은 이미
코드에 있었고, 빠진 것은 `bpe.vocab` 파일과 디코딩 방식 두 가지뿐이었다.

* `DecodingMethod` 기본값 `greedy_search` → `modified_beam_search`
* `bUseHotwords` 기본값 `false` → `true`
* `StartListening`에 넘긴 `Keywords`가 그대로 hotwords가 된다 (기존 배선 그대로)

**`bpe.vocab` 생성**: 업스트림 모델 저장소에는 `bpe.model`만 있고 `bpe.vocab`이 없다.
`Scripts/DownloadKoreanVoiceModel.py`가 `bpe.model`(sentencepiece ModelProto)의 protobuf를
직접 읽어 `piece<TAB>score` 형식으로 생성하도록 했다. sentencepiece 패키지를 설치하지 않아도
파이썬 인터프리터만 있으면 되도록 필드 1(`repeated SentencePiece`)만 걷는 최소 파서를 넣었다.

**폴백 (중요)**: `bpe.vocab`은 **필수 파일이 아니라 선택 파일**로 분류했다.

* `GetRequiredModelFiles()` — encoder/decoder/joiner/tokens 4개만
* `GetOptionalModelFiles()` — `bpe.vocab`
* `UVoiceModelLibrary::ResolveNativeModelDirectory()`에 `OptionalFiles` 인자를 추가해,
  선택 파일은 있으면 Android에서 함께 추출하되 없어도 실패시키지 않는다

따라서 스크립트를 다시 돌리지 않은 다른 개발자의 체크아웃에서도 `ModelMissing`이 되지 않고,
경고를 남긴 뒤 hotwords 없이 `greedy_search`로 동작한다. hotwords가 켜져 있는데
`DecodingMethod`가 다른 값이면 `modified_beam_search`로 강제한다.

실제 적용 결과는 `ssv.voice.status` 출력과 로드 로그에 노출된다.

```text
Korean speech recognizer loaded in 1.30s (..., modified_beam_search, hotwords on, 2 threads).
```

## B. 발화마다 마이크를 재오픈하던 문제

`UVoiceRecognitionComponent`는 발화 1건 = 요청 1건으로 끝나는 단발 구조다. 연속 리스닝 호출부인
`AVoiceKeywordTestActor`는 결과를 받은 **같은 프레임에** 다시 `StartListening`을 부르는데,
이전 구조에서는 그 사이에 `StopCapture()` → `StartCapture()`로 오디오 장치를 닫았다 다시 열었다.
잡음 오인식 1건마다 마이크가 재오픈되므로 그 순간 말을 시작하면 앞음절이 통째로 사라진다.
(선행 작업 로그의 오인식 22건은 곧 마이크 재오픈 22회였다.)

`EndBackendListening`이 장치를 바로 닫는 대신 `CaptureIdleTimeout`(신규, 기본 2초) 타이머를 걸고,
그 안에 새 요청이 오면 타이머를 취소하고 열려 있는 장치를 재사용하도록 바꿨다.

유휴 구간에 녹음이 남지 않도록 캡처 콜백에 `bAcceptSamples` 게이트를 두었다. 요청 밖에서 들어온
샘플은 버퍼에 쌓이지도, 디코더에 들어가지도 않는다. `CaptureIdleTimeout = 0`이면 이전처럼 요청이
끝나는 즉시 닫는다. 월드가 없거나 teardown 중이면 타이머를 걸지 않고 즉시 닫는다.

베이스 클래스 주석의 "Capture is only ever active between StartListening and StopListening"이
이제 부정확해져서, 녹음 구간과 장치 개방 구간을 구분하도록 문구를 고쳤다.

# 변경 파일

```text
Scripts/DownloadKoreanVoiceModel.py                                        (bpe.vocab 생성)
Source/SuwonSiegeContestVR/Public/Core/Voice/SherpaVoiceRecognitionComponent.h
Source/SuwonSiegeContestVR/Private/Core/Voice/SherpaVoiceRecognitionComponent.cpp
Source/SuwonSiegeContestVR/Public/Core/Voice/VoiceModelLibrary.h            (OptionalFiles)
Source/SuwonSiegeContestVR/Private/Core/Voice/VoiceModelLibrary.cpp
Source/SuwonSiegeContestVR/Public/Core/Voice/VoiceRecognitionComponent.h    (주석만)
docs/Core/specs/SHERPA_ONNX_INTEGRATION.md                                 (3·4·8절 수정, 9절 신설)
docs/Core/completed/2026-08-27_VOICE_KEYWORD_TEST_LEVEL.md                 (남은 문제 갱신)
docs/Core/completed/2026-08-27_VOICE_HOTWORDS_AND_CAPTURE.md               (이 문서)
```

Blueprint·레벨·에셋은 건드리지 않았다. 게임플레이 호출부(`UInitialConsonantQuizComponent`,
`AVoiceKeywordTestActor`)도 변경 없이 그대로 이득을 본다.

# 주요 결정 사항

| 결정 | 이유 |
|---|---|
| `bpe.vocab`을 다운로드가 아니라 스크립트에서 생성 | 업스트림 저장소에 그 파일이 없다. sherpa-onnx 공식 `export_bpe_vocab.py`와 같은 출력(`piece<TAB>score`, id 순서)을 만든다 |
| sentencepiece 패키지 대신 protobuf 최소 파서 | 이 스크립트의 존재 이유가 "새 체크아웃에서 파이썬만 있으면 된다"는 것이다. pip 의존성을 새로 추가하고 싶지 않았다 |
| `bpe.vocab`을 필수가 아닌 선택 파일로 분류 | 필수로 두면 스크립트를 다시 돌리지 않은 다른 개발자의 체크아웃이 전부 `ModelMissing`이 된다. 협업 규칙상 남의 환경을 깨지 않는 쪽을 택했다 |
| VAD/endpoint를 끄지 않음 | VAD는 애초에 없었고, endpoint는 오디오를 버리지 않아 원인이 아니다. 오히려 최종 결과 보고가 endpoint에 걸려 있어 끄면 기능이 멈춘다 |
| 마이크를 계속 열어 두는 대신 유휴 타임아웃 + 샘플 게이트 | 장치를 영구히 열어 두면 베이스 클래스가 명시한 "요청 밖에서 녹음하지 않는다"는 계약을 깬다. 게이트를 두면 계약을 지키면서 재오픈 비용만 없앨 수 있다 |

# 테스트 결과

* **Win64 Game(비에디터) 타깃 빌드 성공** — 에디터가 실행 중이라 에디터 타깃 DLL은 잠겨 있었다.
  동일한 코드가 컴파일되므로 컴파일 오류는 이 경로로 확인했다.
* **`bpe.vocab` 생성·검증 완료**: 5000 pieces, 132KB.
  * 형식이 `piece<TAB>score`이고 UTF-8인 것을 확인
  * piece 순서가 `tokens.txt`와 **완전히 동일**함을 확인 (id 정렬이 어긋나면 hotwords가 엉뚱한
    토큰에 매핑된다)
  * "신기전"에 필요한 `▁신`(295) / `기`(59) / `전`(291), "옹성"에 필요한 `성`(379)이 모두 존재
* 스크립트 재실행 시 기존 파일은 `[skip]`, `bpe.vocab`만 `[make]` 되는 동작 확인

# 남은 문제

* **마이크로 "신기전"이 실제로 얼마나 인식되는지 미측정.** 에디터가 실행 중이어서 에디터 타깃
  빌드와 `Suwon.Core.Voice.SherpaKoreanDecode` 자동화 테스트를 돌리지 못했다.
  에디터를 닫고 재빌드한 뒤 `L_VoiceKeywordTest`에서 사람이 직접 발화해 확인해야 한다.
  헤더에 새 `UPROPERTY`가 추가되었으므로 Live Coding이 아니라 **에디터 재시작**이 필요하다.
* `modified_beam_search`는 `greedy_search`보다 느리다. PC에서는 RTF 0.06이라 여유가 크지만,
  **Quest CPU에서의 디코딩 비용은 미측정**이다. 부담되면 `NumThreads`나 `max_active_paths`(현재 4)를
  조정한다.
* `HotwordsScore`는 기본값 2.0을 그대로 뒀다. 부스팅이 약하면 올리고, 반대로 "옹성"·"신기전"이
  아닌 발화까지 그쪽으로 끌려가면 낮춘다. 실측 후 조정할 값이다.
* Android 검증은 여전히 범위 밖(스펙 6절의 NDK 미설치 상태 그대로).
