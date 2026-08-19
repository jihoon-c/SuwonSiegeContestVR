import unreal


def log_property(asset, property_name):
    try:
        unreal.log(f"{property_name}={asset.get_editor_property(property_name)}")
    except Exception as error:
        unreal.log_warning(f"Could not read {property_name}: {error}")


scenario = unreal.load_asset("/Game/Data/DA_Scenario_Singijeon")
table = unreal.load_asset("/Game/Data/DT_Narration")
manager_bp = unreal.load_asset("/Game/Core/Scenario/Managers/BP_ScenarioManager")

unreal.log(f"SCENARIO_INSPECT scenario={scenario} class={scenario.get_class() if scenario else None}")
if scenario:
    for name in ("scenario_id", "start_stage_id", "stages", "narration_table"):
        log_property(scenario, name)
    stages = list(scenario.get_editor_property("stages"))
    stage = stages[0] if stages else None
    for index, interaction in enumerate(stage.get_editor_property("interactions") if stage else []):
        unreal.log(
            "SCENARIO_INTERACTION "
            f"{index}: id={interaction.get_editor_property('interaction_id')} "
            f"type={interaction.get_editor_property('interaction_type')} "
            f"target={interaction.get_editor_property('target_id')} "
            f"narration={interaction.get_editor_property('narration_id')} "
            f"next={interaction.get_editor_property('next_interaction_id')} "
            f"success={interaction.get_editor_property('success_interaction_id')} "
            f"fail={interaction.get_editor_property('fail_interaction_id')}"
        )

unreal.log(f"SCENARIO_INSPECT table={table}")
if table:
    unreal.log(f"SCENARIO_INSPECT rows={unreal.DataTableFunctionLibrary.get_data_table_row_names(table)}")

unreal.log(f"SCENARIO_INSPECT manager_bp={manager_bp}")
if manager_bp and manager_bp.generated_class():
    cdo = unreal.get_default_object(manager_bp.generated_class())
    for name in ("scenario_definition", "narration_table", "b_auto_start_scenario"):
        log_property(cdo, name)
