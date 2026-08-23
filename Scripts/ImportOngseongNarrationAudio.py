import json
import os
from urllib.parse import unquote_plus

import unreal


TABLE_PATH = "/GF_OngseongCrossbow/Data/DT_OngseongNarration"
DESTINATION_PATH = "/GF_OngseongCrossbow/Asset/Narration"
DOWNLOADS_PATH = r"C:\Users\Foryoucom\Downloads"

# The row names are stable asset names; prefixes tolerate browser-shortened filenames.
ROW_TO_PREFIX = {
    "ON_01": "지금부터 성문을 지키는 법을 익히도록 합시다",
    "ON_02": "성문은 성에서 가장 중요한 곳인 동시에",
    "ON_03": "옹성은 성문을 반달 모양으로 감싸는 방어 시설입니다",
    "ON_04": "안으로 들어온 적은 사방의 성벽에 둘러싸이게 되고",
    "ON_05": "즉, 성문을 공격하러 들어온 적을 오히려 한곳에 몰아넣어",
    "ON_06": "총통은 화약의 힘으로 탄환을 발사하는 화포이죠",
    "ON_07": "오늘 지휘관님이 직접 이 총통을 다뤄보게 될 것입니다",
    "ON_08": "지금부터 실제 상황을 가정한 수비 훈련을 시작하겠습니다",
    "ON_09": "지휘관님은 옹성 위의 총통을 맡으십시오",
    "ON_10": "적이 쳐들어오고있습니다!",
    "ON_11": "당황하지 마십시오! 총통을 이용해 적을 물리치는겁니다",
    "ON_12": "먼저 총통을 집어 발사를 준비하십시오",
    "ON_13": "화약을 집어 총통의 포구에 넣으십시오",
    "ON_14": "수시개를 잡고 포구 안으로 세 번 밀어 넣어 화약을 다지십시오",
    "ON_15": "대포알을 집어 총통의 포구에 넣으십시오",
    "ON_16": "화승을 장전부에 가져다 대어 발사를 준비하십시오",
    "ON_17": "좋습니다! 이제 총통을 발사할 수 있습니다",
    "ON_18": "대포알을 모두 발사했습니다",
    "ON_19": "충차가 성문에 다가가고 있습니다! 어서 총통으로 충차를 파괴하십시오",
    "ON_20": "아군이 공격받고 있습니다",
    "ON_21": "성문이 공격받고 있습니다",
    "ON_22": "적의 공세를 성공적으로 막았습니다",
    "ON_23": "성문이 파괴되었습니다",
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
        raise RuntimeError(f"Expected one recording for '{prefix}', found {len(candidates)}: {candidates}")
    return candidates[0]


def import_sound(row_name, source_file):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_file)
    task.set_editor_property("destination_path", DESTINATION_PATH)
    task.set_editor_property("destination_name", row_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    sound = unreal.load_asset(f"{DESTINATION_PATH}/{row_name}")
    if not sound:
        raise RuntimeError(f"Could not import {source_file} as {row_name}")
    return sound


table = unreal.load_asset(TABLE_PATH)
if not table:
    raise RuntimeError(f"Missing {TABLE_PATH}")

exported = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table)
success, payload = exported if isinstance(exported, tuple) else (True, exported)
if not success:
    raise RuntimeError("Could not export DT_OngseongNarration")
rows = json.loads(payload)
rows_by_name = {row["Name"]: row for row in rows}
if set(rows_by_name) != set(ROW_TO_PREFIX):
    raise RuntimeError("Ongseong narration rows do not match the expected ON_01 through ON_23 set")

for row_name, prefix in ROW_TO_PREFIX.items():
    sound = import_sound(row_name, find_source_file(prefix))
    rows_by_name[row_name]["NarrationSound"] = sound.get_path_name()

# Keep the subtitles authoritative even when the asset table existed before the revised script.
rows_by_name["ON_19"]["Subtitle"] = "충차가 성문에 다가가고 있습니다! 어서 총통으로 충차를 파괴하십시오"
rows_by_name["ON_20"]["Subtitle"] = "아군이 공격받고 있습니다. 적 궁병을 처치하여 아군 총통을 보호하십시오"

if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table, json.dumps(rows, ensure_ascii=False)):
    raise RuntimeError("Could not update DT_OngseongNarration")
if not unreal.EditorAssetLibrary.save_loaded_asset(table, only_if_is_dirty=False):
    raise RuntimeError("Could not save DT_OngseongNarration")
unreal.EditorAssetLibrary.save_directory(DESTINATION_PATH, only_if_is_dirty=False, recursive=True)

unreal.log("ONGSEONG_NARRATION_IMPORT SUCCESS: 23 recordings linked and revised subtitles saved")
