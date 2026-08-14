import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
DEFINITION_PATH = "/Game/Core/Experience/Definitions/DA_Experience_Singijeon"
LEVEL_PATH = "/GF_Singijeon/Maps/LV_Singijeon"


def create_or_load_definition():
    definition = unreal.load_asset(DEFINITION_PATH)
    if definition:
        return definition

    definition_class = unreal.load_class(
        None, "/Script/SuwonSiegeContestVR.ExperienceDefinition"
    )
    if not definition_class:
        raise RuntimeError("Could not load UExperienceDefinition")

    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", definition_class)
    definition = ASSET_TOOLS.create_asset(
        "DA_Experience_Singijeon",
        "/Game/Core/Experience/Definitions",
        definition_class,
        factory,
    )
    if not definition:
        raise RuntimeError("Could not create DA_Experience_Singijeon")
    return definition


experience_level = unreal.load_asset(LEVEL_PATH)
if not experience_level:
    raise RuntimeError("Could not load LV_Singijeon")

definition = create_or_load_definition()
definition.set_editor_property("experience_id", "EXP_Singijeon")
definition.set_editor_property("display_name", "신기전")
definition.set_editor_property("experience_level", experience_level)
definition.set_editor_property("return_on_completion", True)
definition.set_editor_property("travel_options", "")
unreal.EditorAssetLibrary.save_loaded_asset(definition, only_if_is_dirty=False)

unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
managers = [
    actor
    for actor in unreal.EditorLevelLibrary.get_all_level_actors()
    if isinstance(actor, unreal.ScenarioManagerActor)
]
if len(managers) != 1:
    raise RuntimeError(
        f"Expected exactly one ScenarioManagerActor in LV_Singijeon, found {len(managers)}"
    )

manager = managers[0]
manager.set_editor_property("experience_definition", definition)
manager.set_editor_property("activate_experience_when_opened_directly", True)
manager.set_editor_property("complete_experience_on_scenario_finished", True)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(
    save_map_packages=True, save_content_packages=True
)
unreal.log("EXPERIENCE_SETUP SUCCESS")
