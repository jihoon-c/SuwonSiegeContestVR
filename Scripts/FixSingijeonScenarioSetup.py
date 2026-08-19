import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
SCENARIO_PATH = "/Game/Data/DA_Scenario_Singijeon"
EXPERIENCE_PATH = "/Game/Core/Experience/Definitions/DA_Experience_Singijeon"
NARRATION_PATH = "/Game/Data/DT_Narration"
LEVEL_PATH = "/Game/Maps/LV_Singijeon"


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


narration_table = unreal.load_asset(NARRATION_PATH)
scenario = create_or_load_scenario()
experience = unreal.load_asset(EXPERIENCE_PATH)
if not narration_table or not experience:
    raise RuntimeError("DT_Narration or DA_Experience_Singijeon is missing")
if not scenario.get_editor_property("stages"):
    raise RuntimeError("DA_Scenario_Singijeon has no inline Stage")
scenario.set_editor_property("scenario_id", "SCENARIO_Singijeon")
scenario.set_editor_property("scenario_name", "Singijeon")
scenario.set_editor_property("narration_table", narration_table)
unreal.EditorAssetLibrary.save_loaded_asset(scenario)
experience.set_editor_property("scenario_definition", scenario)
experience.set_editor_property("auto_start_scenario", True)
experience.set_editor_property("complete_on_scenario_finished", True)
unreal.EditorAssetLibrary.save_loaded_asset(experience)

unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
configured_count = 0
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if isinstance(actor, unreal.ScenarioManagerActor):
        actor.set_editor_property("experience_definition", experience)
        actor.set_editor_property("standalone_scenario_definition", None)
        actor.refresh_resolved_configuration()
        configured_count += 1

if configured_count == 0:
    raise RuntimeError("LV_Singijeon has no ScenarioManagerActor")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(
    save_map_packages=True, save_content_packages=True
)
if not unreal.EditorAssetLibrary.save_asset(LEVEL_PATH, only_if_is_dirty=False):
    raise RuntimeError("Could not force-resave LV_Singijeon")
unreal.log(
    f"Configured {configured_count} ScenarioManagerActor(s) with "
    f"{SCENARIO_PATH} and {NARRATION_PATH}."
)
