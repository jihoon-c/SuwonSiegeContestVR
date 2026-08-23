import json
import os
from urllib.parse import unquote_plus

import unreal


TABLE_PATH = "/GF_Geojunggi/Data/DT_NokroNarration"
DESTINATION_PATH = "/GF_Geojunggi/Asset/Narration"
DOWNLOADS_PATH = r"C:\Users\Foryoucom\Downloads"

# These are the rows for which a recording is currently available in Downloads.
# Matching uses the beginning of the decoded filename to also support the three
# filenames that were shortened by the browser/operating system.
ROW_TO_PREFIX = {
    "NK_01": "무거운 성돌을 높은 곳까지 옮기기 위해서는 사람의 힘을 보다 효율적으로 사용할 수 있는 장비가 필요했습니다.",
    "NK_02": "수원화성 축성에는 이러한 작업을 돕기 위해 녹로와 같은 기구가 사용되었습니다.",
    "NK_03": "녹로는 도르래와 줄을 이용하여 무거운 물체를 들어 올리고 내릴 수 있도록 만든 장치입니다.",
    "NK_04": "지금부터 녹로를 사용하여 파손된 성벽을 직접 보수해 보겠습니다.",
    "NK_05": "주변 성벽의 일부가 파손되어 있습니다.",
    "NK_06": "노란색으로 표시된 부분이 성돌을 배치해야 하는 위치입니다.",
    "NK_07": "녹로를 조작하여 성돌을 들어 올리고, 표시된 위치에 정확하게 배치하십시오.",
    "NK_08": "먼저 녹로의 손잡이를 그랩 버튼으로 잡으십시오.",
    "NK_09": "손잡이를 돌리면 녹로에 연결된 줄이 움직이며 성돌의 높이를 조절할 수 있습니다.",
    "NK_10": "손잡이를 돌려 성돌을 필요한 높이까지 들어 올리십시오.",
    "NK_11": "이제 성돌을 파손된 성벽이 있는 방향으로 이동시켜야 합니다.",
    "NK_12": "조이스틱을 움직여 녹로의 회전 방향을 조절하십시오.",
    "NK_13": "노란색으로 표시된 위치까지 성돌을 이동시키십시오.",
    "NK_14": "성돌의 높이와 위치를 노란색으로 표시된 부분에 정확하게 맞추십시오.",
    "NK_15": "위치를 맞췄다면 양손의 트리거 버튼을 동시에 눌러 성돌을 배치하십시오.",
    "NK_16": "양손 트리거를 함께 눌러 배치를 시도하십시오.",
    "NK_17": "성돌이 정확한 위치에 배치되었습니다.",
    "NK_18": "주변의 다른 파손된 부분도 같은 방법으로 보수하십시오.",
    "NK_19": "손잡이로 높이를 조절하고, 조이스틱으로 방향을 맞춘 뒤 성돌을 배치하십시오.",
    "NK_20": "성돌의 위치가 정확하지 않습니다.",
    "NK_21": "성돌이 처음 위치로 돌아갑니다.",
    "NK_22": "노란색으로 표시된 위치를 다시 확인하고 성돌의 높이와 방향을 정확하게 맞추십시오.",
    "NK_23": "아직 보수해야 할 부분이 남아 있습니다.",
    "NK_24": "노란색으로 표시된 모든 위치에 성돌을 배치하십시오.",
    "NK_25": "마지막으로 남은 파손 부분입니다.",
    "NK_26": "성돌의 위치를 정확하게 맞추고 배치하십시오.",
    "NK_27": "파손된 성벽의 보수가 모두 완료되었습니다.",
    "NK_28": "이번 체험에서는 녹로를 이용하여 무거운 성돌의 높이와 위치를 조절하고, 성벽에 배치하는 과정을 체험했습니다.",
    "NK_29": "녹로와 같은 기구는 무거운 자재를 보다 효율적으로 옮길 수 있도록 하여 수원화성의 축성과 보수 작업을 도왔습니다.",
    "NK_30": "이것으로 녹로를 이용한 성벽 보수 체험을 마치겠습니다.",
    "NK_31": "수원화성과 같이 크고 높은 성벽은 어떻게 쌓고 보수할 수 있었을까요?",
}


def normalise_filename(filename):
    return unquote_plus(os.path.splitext(filename)[0]).rstrip("_").strip()


def find_source_file(prefix):
    candidates = []
    for filename in os.listdir(DOWNLOADS_PATH):
        if not filename.lower().endswith((".wav", ".mp3", ".ogg", ".flac")):
            continue
        decoded = normalise_filename(filename)
        if prefix.startswith(decoded) or decoded.startswith(prefix):
            candidates.append(os.path.join(DOWNLOADS_PATH, filename))
    if len(candidates) != 1:
        raise RuntimeError(f"Expected one recording for '{prefix}', found {len(candidates)}")
    return candidates[0]


def import_sound(row_name, source_file):
    asset_path = f"{DESTINATION_PATH}/{row_name}"
    existing = unreal.load_asset(asset_path)
    if existing:
        return existing

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_file)
    task.set_editor_property("destination_path", DESTINATION_PATH)
    task.set_editor_property("destination_name", row_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", False)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    imported = task.get_editor_property("imported_object_paths")
    if len(imported) != 1:
        raise RuntimeError(f"Failed to import {source_file}: {imported}")
    sound = unreal.load_asset(imported[0])
    if not sound:
        raise RuntimeError(f"Could not load imported asset {imported[0]}")
    return sound


table = unreal.load_asset(TABLE_PATH)
if not table:
    raise RuntimeError("DT_NokroNarration was not found")

rows = json.loads(table.export_to_json_string())
rows_by_name = {row["Name"]: row for row in rows}
linked_rows = []
for row_name, prefix in ROW_TO_PREFIX.items():
    if row_name not in rows_by_name:
        raise RuntimeError(f"Narration row {row_name} was not found")
    sound = import_sound(row_name, find_source_file(prefix))
    rows_by_name[row_name]["NarrationSound"] = sound.get_path_name()
    linked_rows.append(row_name)

if not table.fill_from_json_string(json.dumps(rows, ensure_ascii=False)):
    raise RuntimeError("Could not update DT_NokroNarration")
if not unreal.EditorAssetLibrary.save_loaded_asset(table, only_if_is_dirty=False):
    raise RuntimeError("Could not save DT_NokroNarration")
unreal.EditorAssetLibrary.save_directory(DESTINATION_PATH, only_if_is_dirty=False, recursive=True)

unreal.log(f"NOKRO_NARRATION_IMPORT SUCCESS: {len(linked_rows)} recordings linked ({', '.join(linked_rows)})")
