"""Downloads the on-device Korean speech model used by the initial-consonant quiz.

The model is ~140MB, so it is not committed. Run this once per checkout:

    python Scripts/DownloadKoreanVoiceModel.py

Files land in <Project>/VoiceModels/<model>/ , which DefaultGame.ini stages as non-UFS content
for packaged Windows and Android builds. Re-running only fetches what is missing or truncated.

The script also derives bpe.vocab from bpe.model. sherpa-onnx needs that file to turn hotwords
into BPE units, and the upstream model package only ships bpe.model.

See docs/Core/specs/SHERPA_ONNX_INTEGRATION.md.
"""

import argparse
import os
import struct
import sys
import urllib.error
import urllib.request

MODEL_NAME = "sherpa-onnx-streaming-zipformer-korean-2024-06-16"
BASE_URL = f"https://huggingface.co/k2-fsa/{MODEL_NAME}/resolve/main"

# (relative path, approximate size in bytes) - the size guards against truncated downloads.
FILES = [
    ("encoder-epoch-99-avg-1.int8.onnx", 127_000_000),
    ("decoder-epoch-99-avg-1.onnx", 11_000_000),
    ("joiner-epoch-99-avg-1.int8.onnx", 2_500_000),
    ("tokens.txt", 50_000),
    ("bpe.model", 300_000),
    # Used by the automation test so the backend can be verified without a microphone.
    ("test_wavs/0.wav", 50_000),
    ("test_wavs/1.wav", 50_000),
    ("test_wavs/trans.txt", 100),
]

MIN_SIZE_RATIO = 0.8

# Derived locally rather than downloaded: the upstream repository ships no bpe.vocab.
BPE_MODEL = "bpe.model"
BPE_VOCAB = "bpe.vocab"


def project_root() -> str:
    return os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def download(url: str, destination: str) -> None:
    os.makedirs(os.path.dirname(destination), exist_ok=True)
    temporary = destination + ".part"

    with urllib.request.urlopen(url, timeout=60) as response, open(temporary, "wb") as out:
        total = int(response.headers.get("Content-Length") or 0)
        written = 0
        while True:
            chunk = response.read(1 << 20)
            if not chunk:
                break
            out.write(chunk)
            written += len(chunk)
            if total:
                percent = 100.0 * written / total
                print(f"\r  {percent:5.1f}%  {written / 1e6:7.1f} / {total / 1e6:.1f} MB", end="")
        print()

    os.replace(temporary, destination)


def _read_varint(data: bytes, index: int):
    result = 0
    shift = 0
    while True:
        byte = data[index]
        index += 1
        result |= (byte & 0x7F) << shift
        if not byte & 0x80:
            return result, index
        shift += 7


def _read_sentence_piece(payload: bytes):
    """Reads one SentencePiece submessage: {string piece = 1, float score = 2, enum type = 3}."""
    index = 0
    piece = None
    score = 0.0
    while index < len(payload):
        key, index = _read_varint(payload, index)
        field, wire = key >> 3, key & 7
        if wire == 0:
            _, index = _read_varint(payload, index)
        elif wire == 5:
            if field == 2:
                score = struct.unpack("<f", payload[index:index + 4])[0]
            index += 4
        elif wire == 2:
            length, index = _read_varint(payload, index)
            if field == 1:
                piece = payload[index:index + length].decode("utf-8")
            index += length
        elif wire == 1:
            index += 8
        else:
            raise ValueError(f"unsupported wire type {wire}")
    return piece, score


def read_bpe_pieces(bpe_model_path: str):
    """Extracts (piece, score) pairs from a sentencepiece bpe.model, in vocabulary id order.

    bpe.model is a protobuf ModelProto whose field 1 repeats SentencePiece. Only that field is
    needed, so it is walked directly rather than pulling in the sentencepiece package: the point of
    this script is that a fresh checkout needs nothing but a Python interpreter.
    """
    with open(bpe_model_path, "rb") as model_file:
        data = model_file.read()

    index = 0
    pieces = []
    while index < len(data):
        key, index = _read_varint(data, index)
        field, wire = key >> 3, key & 7
        if wire == 2:
            length, index = _read_varint(data, index)
            if field == 1:
                pieces.append(_read_sentence_piece(data[index:index + length]))
            index += length
        elif wire == 0:
            _, index = _read_varint(data, index)
        elif wire == 5:
            index += 4
        elif wire == 1:
            index += 8
        else:
            raise ValueError(f"unsupported wire type {wire}")
    return pieces


def write_bpe_vocab(model_dir: str, force: bool) -> bool:
    """Writes bpe.vocab beside bpe.model in the "piece<TAB>score" form sherpa-onnx expects.

    Without it sherpa-onnx cannot map a hotword such as "신기전" onto BPE units, and hotwords are
    what makes multi-syllable answers survive decoding.
    """
    bpe_model_path = os.path.join(model_dir, BPE_MODEL)
    vocab_path = os.path.join(model_dir, BPE_VOCAB)

    if not os.path.exists(bpe_model_path):
        print(f"[warn] {BPE_MODEL} is missing, so hotwords stay unavailable.", file=sys.stderr)
        return False
    if not force and os.path.exists(vocab_path) and os.path.getsize(vocab_path) > 0:
        print(f"[skip] {BPE_VOCAB}")
        return True

    try:
        pieces = read_bpe_pieces(bpe_model_path)
    except (ValueError, IndexError, UnicodeDecodeError) as error:
        print(f"[warn] Could not read {BPE_MODEL}: {error}", file=sys.stderr)
        return False

    if not pieces:
        print(f"[warn] {BPE_MODEL} held no vocabulary entries.", file=sys.stderr)
        return False

    temporary = vocab_path + ".part"
    with open(temporary, "w", encoding="utf-8", newline="\n") as out:
        for piece, score in pieces:
            out.write(f"{piece}\t{score}\n")
    os.replace(temporary, vocab_path)
    print(f"[make] {BPE_VOCAB} ({len(pieces)} pieces)")
    return True


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--force", action="store_true", help="re-download files that already exist")
    parser.add_argument("--output", default=None, help="target directory (defaults to <Project>/VoiceModels)")
    args = parser.parse_args()

    root = args.output or os.path.join(project_root(), "VoiceModels")
    model_dir = os.path.join(root, MODEL_NAME)
    print(f"Model directory: {model_dir}")

    for relative, expected_size in FILES:
        destination = os.path.join(model_dir, relative.replace("/", os.sep))
        if not args.force and os.path.exists(destination):
            actual = os.path.getsize(destination)
            if actual >= expected_size * MIN_SIZE_RATIO:
                print(f"[skip] {relative} ({actual / 1e6:.1f} MB)")
                continue
            print(f"[redo] {relative} looks truncated ({actual / 1e6:.1f} MB)")

        url = f"{BASE_URL}/{relative}"
        print(f"[get ] {relative}")
        try:
            download(url, destination)
        except (urllib.error.URLError, urllib.error.HTTPError) as error:
            print(f"Failed to download {url}: {error}", file=sys.stderr)
            return 1

    write_bpe_vocab(model_dir, args.force)

    print("\nDone. The quiz uses this model automatically on the next Play.")
    print("Windows: nothing else to do. Android: the files are staged into the package and")
    print("extracted to the device on first use.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
