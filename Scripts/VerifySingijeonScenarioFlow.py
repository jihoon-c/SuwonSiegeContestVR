import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_FLOW_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_FLOW_VERIFY FAIL: {message}")


scenario = unreal.load_asset("/Game/Data/DA_Scenario_Singijeon")
table = unreal.load_asset("/Game/Data/DT_Narration")
check(scenario is not None, "DA_Scenario_Singijeon loads")
check(table is not None, "DT_Narration loads")

expected_next = {
    "NAR_01": "NAR_02", "NAR_02": "NAR_03", "NAR_03": "NAR_04",
    "NAR_04": "NAR_05", "NAR_05": "INT_01", "INT_01": "NAR_06",
    "NAR_06": "NAR_07", "NAR_07": "NAR_08", "NAR_08": "INT_02",
    "INT_02": "NAR_09", "NAR_09": "NAR_10", "NAR_10": "INT_03",
    "INT_03": "NAR_11", "NAR_11": "NAR_12", "NAR_12": "INT_04",
    "INT_04": "NAR_13", "NAR_13": "INT_05", "INT_05": "NAR_14",
    "NAR_14": "NAR_15", "NAR_15": "INT_06", "INT_06": "NAR_16",
    "NAR_16": "INT_07", "INT_07": "NAR_17", "NAR_17": "NAR_18",
    "NAR_18": "NAR_19", "NAR_19": "NAR_20", "NAR_20": "NAR_21",
    "NAR_21": "None",
}
expected_guides = {
    "INT_01": unreal.ScenarioGuideAction.GRAB,
    "INT_02": unreal.ScenarioGuideAction.DRAG,
    "INT_03": unreal.ScenarioGuideAction.DRAG,
    "INT_04": unreal.ScenarioGuideAction.GRAB,
    "INT_05": unreal.ScenarioGuideAction.DRAG,
    "INT_06": unreal.ScenarioGuideAction.DRAG,
    "INT_07": unreal.ScenarioGuideAction.HIDDEN,
}

if scenario:
    stages = list(scenario.get_editor_property("stages"))
    check(len(stages) == 1, "Scenario contains one inline Stage")
    stage = stages[0] if stages else None
    interactions = list(stage.get_editor_property("interactions")) if stage else []
    by_id = {
        str(interaction.get_editor_property("interaction_id")): interaction
        for interaction in interactions
    }
    check(
        stage is not None and str(stage.get_editor_property("start_interaction_id")) == "NAR_01",
        "Stage starts at NAR_01",
    )
    check(len(interactions) == 28, "Stage contains 21 narration and 7 gameplay interactions")
    for interaction_id, next_id in expected_next.items():
        interaction = by_id.get(interaction_id)
        check(interaction is not None, f"{interaction_id} exists")
        if interaction:
            actual_next = str(interaction.get_editor_property("next_interaction_id"))
            check(actual_next == next_id, f"{interaction_id} advances to {next_id}")
    for interaction_id, guide_action in expected_guides.items():
        interaction = by_id.get(interaction_id)
        if interaction:
            check(interaction.get_editor_property("guide_action") == guide_action,
                  f"{interaction_id} uses its authored input guide")
            if guide_action != unreal.ScenarioGuideAction.HIDDEN:
                check(bool(str(interaction.get_editor_property("guide_text"))),
                      f"{interaction_id} has a contextual guide instruction")

if table:
    next_rows = unreal.DataTableFunctionLibrary.get_data_table_column_as_string(
        table, "NextRow"
    )
    advance_modes = unreal.DataTableFunctionLibrary.get_data_table_column_as_string(
        table, "AdvanceMode"
    )
    check(all(value == "None" for value in next_rows), "All DT NextRow values are None")
    check(all(value == "Stop" for value in advance_modes), "All DT AdvanceMode values are Stop")

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
check(world is not None, "Moved LV_Singijeon loads from /Game/Maps")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
managers = [actor for actor in actors if isinstance(actor, unreal.ScenarioManagerActor)]
check(len(managers) == 1, "Moved LV_Singijeon contains exactly one Scenario Manager")
if managers:
    check(
        managers[0].get_editor_property("narration_table") == table,
        "Scenario Manager references DT_Narration",
    )

if errors:
    raise RuntimeError("Singijeon flow verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_FLOW_VERIFY SUCCESS")
