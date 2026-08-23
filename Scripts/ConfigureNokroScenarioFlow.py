import json

import unreal


TABLE_PATH = "/GF_Geojunggi/Data/DT_NokroNarration"

# Each interaction event starts one self-contained narration group. NK_31 is the
# opening question; NK_27-NK_30 are the completion explanation.
GROUPS = (
    ("NK_31", "NK_01", "NK_02", "NK_03", "NK_04", "NK_05", "NK_06", "NK_07", "NK_08"),
    ("NK_09", "NK_10", "NK_11"),
    ("NK_12", "NK_13", "NK_14", "NK_15"),
    ("NK_16",),
    ("NK_17", "NK_18", "NK_19"),
    ("NK_20", "NK_21", "NK_22"),
    ("NK_23", "NK_24"),
    ("NK_25", "NK_26"),
    ("NK_27", "NK_28", "NK_29", "NK_30"),
)

table = unreal.load_asset(TABLE_PATH)
if not table:
    raise RuntimeError(f"Missing narration table: {TABLE_PATH}")

rows = json.loads(table.export_to_json_string())
rows_by_name = {row["Name"]: row for row in rows}
expected_names = {f"NK_{index:02d}" for index in range(1, 32)}
if set(rows_by_name) != expected_names:
    missing = sorted(expected_names - set(rows_by_name))
    extra = sorted(set(rows_by_name) - expected_names)
    raise RuntimeError(f"Unexpected Nokro rows. Missing={missing}, Extra={extra}")

for row in rows:
    row["NextRow"] = "None"
    row["AdvanceMode"] = "Auto"

for group in GROUPS:
    for current_name, next_name in zip(group, group[1:]):
        rows_by_name[current_name]["NextRow"] = next_name

if not table.fill_from_json_string(json.dumps(rows, ensure_ascii=False)):
    raise RuntimeError("Could not update DT_NokroNarration")
if not unreal.EditorAssetLibrary.save_loaded_asset(table, only_if_is_dirty=False):
    raise RuntimeError("Could not save DT_NokroNarration")

unreal.log("NOKRO_FLOW_CONFIG_SUCCESS: all 31 rows mapped to interaction-gated groups")
