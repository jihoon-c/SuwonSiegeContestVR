"""Downloads the on-device Korean speech model used by the initial-consonant quiz.

The model is ~140MB, so it is not committed. Run this once per checkout:

    python Scripts/DownloadKoreanVoiceModel.py

Files land in <Project>/VoiceModels/<model>/ , which DefaultGame.ini stages as non-UFS content
for packaged Windows and Android builds. Re-running only fetches what is missing or truncated.

See docs/Core/specs/SHERPA_ONNX_INTEGRATION.md.
"""

import argparse
import os
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

    print("\nDone. The quiz uses this model automatically on the next Play.")
    print("Windows: nothing else to do. Android: the files are staged into the package and")
    print("extracted to the device on first use.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
