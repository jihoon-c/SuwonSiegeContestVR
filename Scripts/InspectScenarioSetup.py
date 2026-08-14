import unreal


def log_property(asset, property_name):
    try:
        unreal.log(f"{property_name}={asset.get_editor_property(property_name)}")
    except Exception as error:
        unreal.log_warning(f"Could not read {property_name}: {error}")


scene = unreal.load_asset("/Game/Data/DA_Scene_Singijeon")
table = unreal.load_asset("/Game/Data/DT_Narration")
manager_bp = unreal.load_asset("/Game/Core/Scenario/Managers/BP_ScenarioManager")

unreal.log(f"SCENARIO_INSPECT scene={scene} class={scene.get_class() if scene else None}")
if scene:
    for name in ("scene_id", "scene_name", "start_interaction_id", "next_scene_id", "interactions"):
        log_property(scene, name)
    for index, interaction in enumerate(scene.get_editor_property("interactions")):
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
