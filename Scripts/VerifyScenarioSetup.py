import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SCENARIO_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SCENARIO_VERIFY FAIL: {message}")


scenario = unreal.load_asset("/Game/Data/DA_Scenario_Singijeon")
narration_table = unreal.load_asset("/Game/Data/DT_Narration")

check(scenario is not None, "DA_Scenario_Singijeon loads")
check(narration_table is not None, "DT_Narration loads")

if scenario:
    stages = list(scenario.get_editor_property("stages"))
    check(len(stages) == 1, "Scenario contains one inline Stage")
    stage = stages[0] if stages else None
    stage_id = stage.get_editor_property("stage_id") if stage else "None"
    start_interaction_id = stage.get_editor_property("start_interaction_id") if stage else "None"
    interactions = list(stage.get_editor_property("interactions")) if stage else []

    check(bool(stage_id), "StageID is assigned")
    check(bool(start_interaction_id), "StartInteractionID is assigned")
    check(bool(interactions), "Stage contains at least one Interaction")
    check(
        scenario.get_editor_property("start_stage_id") == stage_id,
        "Scenario StartStageID matches StageID",
    )

    interaction_ids = [
        interaction.get_editor_property("interaction_id")
        for interaction in interactions
    ]
    check(
        start_interaction_id in interaction_ids,
        "StartInteractionID references an existing Interaction",
    )

    row_names = (
        unreal.DataTableFunctionLibrary.get_data_table_row_names(narration_table)
        if narration_table
        else []
    )
    for interaction in interactions:
        interaction_type = interaction.get_editor_property("interaction_type")
        if interaction_type == unreal.ScenarioInteractionType.NARRATION:
            narration_id = interaction.get_editor_property("narration_id")
            check(
                narration_id in row_names,
                f"NarrationID {narration_id} exists in DT_Narration",
            )

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
managers = [
    actor
    for actor in unreal.EditorLevelLibrary.get_all_level_actors()
    if isinstance(actor, unreal.ScenarioManagerActor)
]
check(len(managers) == 1, "LV_Singijeon contains exactly one Scenario Manager")
if managers:
    manager = managers[0]
    check(
        manager.get_editor_property("scenario_definition") == scenario,
        "Placed Manager references DA_Scenario_Singijeon",
    )
    check(
        manager.get_editor_property("narration_table") == narration_table,
        "Placed Manager references DT_Narration",
    )
    check(
        manager.get_editor_property("auto_start_scenario"),
        "Placed Manager Auto Start Scenario is enabled",
    )

if errors:
    raise RuntimeError("Scenario verification failed: " + "; ".join(errors))

unreal.log("SCENARIO_VERIFY SUCCESS")
