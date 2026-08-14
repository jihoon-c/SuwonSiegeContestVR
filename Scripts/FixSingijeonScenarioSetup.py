import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
SCENE_PATH = "/Game/Data/DA_Scene_Singijeon"
SCENARIO_PATH = "/Game/Data/DA_Scenario_Singijeon"
NARRATION_PATH = "/Game/Data/DT_Narration"
LEVEL_PATH = "/GF_Singijeon/Maps/LV_Singijeon"


def create_or_load_scenario():
    scenario = unreal.load_asset(SCENARIO_PATH)
    if scenario:
        return scenario

    scenario_class = unreal.load_class(
        None, "/Script/SuwonSiegeContestVR.ScenarioDefinition"
    )
    if not scenario_class:
        raise RuntimeError("Could not load UScenarioDefinition")

    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", scenario_class)
    scenario = ASSET_TOOLS.create_asset(
        "DA_Scenario_Singijeon", "/Game/Data", scenario_class, factory
    )
    if not scenario:
        raise RuntimeError("Could not create DA_Scenario_Singijeon")
    return scenario


scene = unreal.load_asset(SCENE_PATH)
narration_table = unreal.load_asset(NARRATION_PATH)
if not scene or not narration_table:
    raise RuntimeError("DA_Scene_Singijeon or DT_Narration is missing")

interactions = list(scene.get_editor_property("interactions"))
if not interactions:
    interaction = unreal.ScenarioInteraction()
    interaction.set_editor_property("interaction_id", "singijeon")
    interaction.set_editor_property("interaction_type", unreal.ScenarioInteractionType.NARRATION)
    interaction.set_editor_property("narration_id", "NewRow")
    interaction.set_editor_property("required", True)
    interactions.append(interaction)
    scene.set_editor_property("interactions", interactions)

scene.set_editor_property("scene_id", "Singijeon")
scene.set_editor_property("start_interaction_id", "singijeon")
unreal.EditorAssetLibrary.save_loaded_asset(scene)

scenario = create_or_load_scenario()
scenario.set_editor_property("scenario_id", "SCENARIO_Singijeon")
scenario.set_editor_property("scenario_name", "Singijeon")
scenario.set_editor_property("scenes", [scene])
scenario.set_editor_property("start_scene_id", "Singijeon")
unreal.EditorAssetLibrary.save_loaded_asset(scenario)

unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
configured_count = 0
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if isinstance(actor, unreal.ScenarioManagerActor):
        actor.set_editor_property("scenario_definition", scenario)
        actor.set_editor_property("narration_table", narration_table)
        actor.set_editor_property("auto_start_scenario", True)
        configured_count += 1

if configured_count == 0:
    raise RuntimeError("LV_Singijeon has no ScenarioManagerActor")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(
    save_map_packages=True, save_content_packages=True
)
unreal.log(
    f"Configured {configured_count} ScenarioManagerActor(s) with "
    f"{SCENARIO_PATH} and {NARRATION_PATH}."
)
