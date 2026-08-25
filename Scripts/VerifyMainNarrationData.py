import json

import unreal


NARRATION_TABLE_PATH = "/Game/Audio/Narration/DT_Narration_Main"
SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"
EXPECTED_START_ROWS = {
    "INTRO_01": "MAIN_NA_01",
    "DEFENSE_01": "MAIN_NA_04",
    "SINGIJEON_BRIEF": "MAIN_NA_08",
    "GONG_SITUATION": "MAIN_NA_09",
    "GONG_ANSWER": "MAIN_NA_12",
    "GONG_IMAGE_01": "MAIN_NA_14",
    "GONG_TRAVEL_BRIEF": "MAIN_NA_15",
    "ONG_SITUATION": "MAIN_NA_17",
    "ONG_ANSWER": "MAIN_NA_21",
    "ONG_IMAGE_02": "MAIN_NA_23",
    "ONG_TRAVEL_BRIEF": "MAIN_NA_25",
    "NOKRO_SITUATION": "MAIN_NA_27",
    "NOKRO_ANSWER": "MAIN_NA_31",
    "NOKRO_IMAGE_02": "MAIN_NA_33",
}

table = unreal.load_asset(NARRATION_TABLE_PATH)
scenario = unreal.load_asset(SCENARIO_PATH)
if not table or not isinstance(scenario, unreal.MainEducationScenarioDefinition):
    raise RuntimeError("Main narration table or Scenario is missing")
if scenario.get_editor_property("narration_table") != table:
    raise RuntimeError("DA_Scenario_MainEducation does not reference DT_Narration_Main")

rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table))
if len(rows) != 33:
    raise RuntimeError(f"Expected 33 Main narration rows, found {len(rows)}")
rows_by_name = {row["Name"]: row for row in rows}
for index in range(1, 34):
    name = f"MAIN_NA_{index:02d}"
    row = rows_by_name.get(name)
    if not row:
        raise RuntimeError(f"Missing row {name}")
    sound_path = str(row.get("NarrationSound", ""))
    if f"/{index:02d}_" not in sound_path:
        raise RuntimeError(f"{name} points to the wrong SoundWave: {sound_path}")

actual_starts = {}
for stage in scenario.get_editor_property("stages"):
    for interaction in stage.get_editor_property("interactions"):
        if interaction.get_editor_property("interaction_type") == unreal.ScenarioInteractionType.NARRATION:
            actual_starts[str(interaction.get_editor_property("interaction_id"))] = str(
                interaction.get_editor_property("narration_id")
            )
if actual_starts != EXPECTED_START_ROWS:
    raise RuntimeError(
        f"Scenario narration starts differ. Expected={EXPECTED_START_ROWS}, Actual={actual_starts}"
    )

for interaction_id, row_name in actual_starts.items():
    if row_name not in rows_by_name:
        raise RuntimeError(f"{interaction_id} references missing row {row_name}")

unreal.log(
    "MAIN_NARRATION VERIFY SUCCESS: 33 rows, 33 SoundWaves, "
    f"{len(actual_starts)} scenario narration segments"
)

