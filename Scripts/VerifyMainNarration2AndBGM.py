"""Read-only verification for Main level BGM and Narration2 scenario wiring."""

import json

import unreal


MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"
TABLE_PATH = "/Game/Audio/Narration/DT_Narration_Main"
SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"
EXPECTED_STARTS = [
    "MAIN_NA_01", "MAIN_NA_02", "MAIN_NA_05", "MAIN_NA_08",
    "MAIN_NA_13", "MAIN_NA_18", "MAIN_NA_25",
]


table = unreal.load_asset(TABLE_PATH)
if not table:
    raise RuntimeError("Narration table is missing")

rows = table.get_row_names()
expected_rows = ["MAIN_NA_{:02d}".format(index) for index in range(1, 31)]
if [str(row) for row in rows] != expected_rows:
    raise RuntimeError("Narration table rows are not exactly MAIN_NA_01 through MAIN_NA_30")

rows_by_name = {
    row["Name"]: row
    for row in json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table))
}
for row_name in expected_rows:
    sound_path = rows_by_name[row_name]["NarrationSound"]
    if not sound_path.startswith("/Game/Audio/Narration2/"):
        raise RuntimeError("{} does not point to Narration2: {}".format(row_name, sound_path))

scenario = unreal.load_asset(SCENARIO_PATH)
if not scenario:
    raise RuntimeError("Main education scenario is missing")
narration_ids = []
for stage in scenario.get_editor_property("stages"):
    for interaction in stage.interactions:
        narration_id = str(interaction.narration_id)
        if narration_id not in ("", "None"):
            narration_ids.append(narration_id)
if narration_ids != EXPECTED_STARTS:
    raise RuntimeError("Scenario narration starts mismatch: {}".format(narration_ids))

unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
players = [actor for actor in actors if isinstance(actor, unreal.MainLevelBGMPlayerActor)]
if len(players) != 1:
    raise RuntimeError("Expected one MainLevelBGMPlayerActor, got {}".format(len(players)))

player = players[0]
music = player.get_editor_property("music")
if not music:
    raise RuntimeError("BGM player has no Music asset")
if not player.get_editor_property("play_on_begin_play"):
    raise RuntimeError("BGM player is not enabled for BeginPlay")

unreal.log("MAIN_NARRATION2_BGM_VERIFY_SUCCESS: rows=30 starts={} music={} begin_play={}".format(
    narration_ids, music.get_path_name(), player.get_editor_property("play_on_begin_play")))
