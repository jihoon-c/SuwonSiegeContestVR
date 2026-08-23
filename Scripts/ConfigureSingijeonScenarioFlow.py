import json
import unreal


SCENARIO_PATH = "/Game/Data/DA_Scenario_Singijeon"
NARRATION_TABLE_PATH = "/Game/Data/DT_Narration"


def make_narration(index, next_id):
    interaction = unreal.ScenarioInteraction()
    interaction.set_editor_property("interaction_id", f"NAR_{index:02d}")
    interaction.set_editor_property(
        "interaction_type", unreal.ScenarioInteractionType.NARRATION
    )
    interaction.set_editor_property("narration_id", f"NA_{index:02d}")
    interaction.set_editor_property("next_interaction_id", next_id)
    interaction.set_editor_property("required", True)
    interaction.set_editor_property("complete_on_start", False)
    return interaction


scenario = unreal.load_asset(SCENARIO_PATH)
narration_table = unreal.load_asset(NARRATION_TABLE_PATH)
if not scenario or not narration_table:
    raise RuntimeError("DA_Scenario_Singijeon or DT_Narration is missing")
stages = list(scenario.get_editor_property("stages"))
if len(stages) != 1:
    raise RuntimeError("DA_Scenario_Singijeon must contain exactly one inline Stage")
stage = stages[0]

existing = {
    str(interaction.get_editor_property("interaction_id")): interaction
    for interaction in list(stage.get_editor_property("interactions"))
}

gameplay_ids = [f"INT_{index:02d}" for index in range(1, 8)]
missing_gameplay_ids = [interaction_id for interaction_id in gameplay_ids if interaction_id not in existing]
if missing_gameplay_ids:
    raise RuntimeError(
        "Missing existing gameplay interactions: " + ", ".join(missing_gameplay_ids)
    )

next_by_gameplay_id = {
    "INT_01": "NAR_06",
    "INT_02": "NAR_09",
    "INT_03": "NAR_11",
    "INT_04": "NAR_13",
    "INT_05": "NAR_14",
    "INT_06": "NAR_16",
    "INT_07": "NAR_17",
}
guide_by_gameplay_id = {
    "INT_01": (unreal.ScenarioGuideAction.GRAB, "신기전을 집으세요."),
    "INT_02": (unreal.ScenarioGuideAction.DRAG, "신기전을 화차 장전 위치로 가져가세요."),
    "INT_03": (unreal.ScenarioGuideAction.DRAG, "화차 손잡이를 잡고 표시된 위치로 이동하세요."),
    "INT_04": (unreal.ScenarioGuideAction.GRAB, "횃불을 집으세요."),
    "INT_05": (unreal.ScenarioGuideAction.DRAG, "횃불을 화로의 불에 가져가세요."),
    "INT_06": (unreal.ScenarioGuideAction.DRAG, "불붙은 횃불을 신기전 도화선에 가져가세요."),
    # 발사는 도화선 점화 후 자동 진행되므로 별도 입력 가이드를 표시하지 않는다.
    "INT_07": (unreal.ScenarioGuideAction.HIDDEN, ""),
}
for interaction_id in gameplay_ids:
    interaction = existing[interaction_id]
    interaction.set_editor_property(
        "next_interaction_id", next_by_gameplay_id[interaction_id]
    )
    interaction.set_editor_property("success_interaction_id", "")
    interaction.set_editor_property("fail_interaction_id", interaction_id)
    guide_action, guide_text = guide_by_gameplay_id[interaction_id]
    interaction.set_editor_property("guide_action", guide_action)
    interaction.set_editor_property("guide_text", guide_text)

next_by_narration_index = {
    1: "NAR_02",
    2: "NAR_03",
    3: "NAR_04",
    4: "NAR_05",
    5: "INT_01",
    6: "NAR_07",
    7: "NAR_08",
    8: "INT_02",
    9: "NAR_10",
    10: "INT_03",
    11: "NAR_12",
    12: "INT_04",
    13: "INT_05",
    14: "NAR_15",
    15: "INT_06",
    16: "INT_07",
    17: "NAR_18",
    18: "NAR_19",
    19: "NAR_20",
    20: "NAR_21",
    21: "",
}

ordered_interactions = []
for index in range(1, 22):
    ordered_interactions.append(make_narration(index, next_by_narration_index[index]))
    gameplay_id = {
        5: "INT_01",
        8: "INT_02",
        10: "INT_03",
        12: "INT_04",
        13: "INT_05",
        15: "INT_06",
        16: "INT_07",
    }.get(index)
    if gameplay_id:
        ordered_interactions.append(existing[gameplay_id])

stage.set_editor_property("start_interaction_id", "NAR_01")
stage.set_editor_property("interactions", ordered_interactions)
scenario.set_editor_property("stages", [stage])
if not unreal.EditorAssetLibrary.save_loaded_asset(scenario, only_if_is_dirty=False):
    raise RuntimeError(
        "Could not save DA_Scenario_Singijeon. Close the asset in other Unreal Editor instances and retry."
    )

export_result = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(
    narration_table
)
if isinstance(export_result, tuple):
    export_success, narration_json = export_result
else:
    export_success, narration_json = True, export_result
if not export_success or not narration_json:
    raise RuntimeError("Could not export DT_Narration to JSON")

rows = json.loads(narration_json)
for row in rows:
    row["NextRow"] = "None"
    row["AdvanceMode"] = "Stop"

import_result = unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(
    narration_table, json.dumps(rows, ensure_ascii=False)
)
if not import_result:
    raise RuntimeError("Could not import the normalized DT_Narration JSON")

if not unreal.EditorAssetLibrary.save_loaded_asset(
    narration_table, only_if_is_dirty=False
):
    raise RuntimeError("Could not save DT_Narration")
unreal.log("SINGIJEON_SCENARIO_FLOW_SETUP SUCCESS")
