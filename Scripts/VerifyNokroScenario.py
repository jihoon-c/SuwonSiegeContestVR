import json

import unreal


TABLE_PATH = "/GF_Geojunggi/Data/DT_NokroNarration"
MAP_PATH = "/GF_Geojunggi/Maps/LV_Nokro"

EXPECTED_NEXT = {
    "NK_31": "NK_01", "NK_01": "NK_02", "NK_02": "NK_03", "NK_03": "NK_04",
    "NK_04": "NK_05", "NK_05": "NK_06", "NK_06": "NK_07", "NK_07": "NK_08",
    "NK_09": "NK_10", "NK_10": "NK_11",
    "NK_12": "NK_13", "NK_13": "NK_14", "NK_14": "NK_15",
    "NK_17": "NK_18", "NK_18": "NK_19",
    "NK_20": "NK_21", "NK_21": "NK_22",
    "NK_23": "NK_24", "NK_25": "NK_26",
    "NK_27": "NK_28", "NK_28": "NK_29", "NK_29": "NK_30",
}

table = unreal.load_asset(TABLE_PATH)
if not table:
    raise RuntimeError(f"Missing narration table: {TABLE_PATH}")
rows = json.loads(table.export_to_json_string())
if len(rows) != 31:
    raise RuntimeError(f"Expected 31 narration rows, found {len(rows)}")

for row in rows:
    name = row["Name"]
    expected_next = EXPECTED_NEXT.get(name, "None")
    if row.get("NextRow", "None") != expected_next:
        raise RuntimeError(f"Broken flow {name}: expected {expected_next}, got {row.get('NextRow')}")
    if not row.get("NarrationSound"):
        raise RuntimeError(f"Narration sound is not assigned: {name}")

if not unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH):
    raise RuntimeError(f"Could not load Nokro map: {MAP_PATH}")

for asset_name in ("BP_NokroCrane", "BP_NokroRepairTarget", "BP_NokroScenarioManager"):
    blueprint = unreal.load_asset(f"/GF_Geojunggi/Gameplay/{asset_name}")
    if not blueprint or not blueprint.generated_class():
        raise RuntimeError(f"Missing or invalid art preset: {asset_name}")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
managers = [
    actor for actor in actor_subsystem.get_all_level_actors()
    if "NokroScenarioManager" in actor.get_class().get_name()
]
if len(managers) != 1:
    raise RuntimeError(f"Expected one Nokro scenario manager in LV_Nokro, found {len(managers)}")

unreal.log("NOKRO_SCENARIO_VERIFY_SUCCESS: map manager, art presets, 31 sounds, and narration gates are valid")
