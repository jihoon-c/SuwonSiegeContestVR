import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"
SINGIJEON_LEVEL_PATH = "/Game/Maps/LV_Singijeon"
MAIN_SCENARIO_PATH = "/Game/Data/DA_Scenario_Main"
MAIN_EXPERIENCE_PATH = "/Game/Core/Experience/Definitions/DA_Experience_Main"
SINGIJEON_EXPERIENCE_PATH = "/Game/Core/Experience/Definitions/DA_Experience_Singijeon"


def create_or_load_data_asset(asset_path, class_path):
    asset = unreal.load_asset(asset_path)
    if asset:
        return asset

    asset_class = unreal.load_class(None, class_path)
    if not asset_class:
        raise RuntimeError(f"Could not load {class_path}")
    package_path, asset_name = asset_path.rsplit("/", 1)
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", asset_class)
    asset = ASSET_TOOLS.create_asset(asset_name, package_path, asset_class, factory)
    if not asset:
        raise RuntimeError(f"Could not create {asset_path}")
    return asset


def make_interaction(interaction_id, interaction_type, next_id="None",
                     target_id="None", objective_text="", complete_on_start=False):
    interaction = unreal.ScenarioInteraction()
    interaction.set_editor_property("interaction_id", interaction_id)
    interaction.set_editor_property("interaction_type", interaction_type)
    interaction.set_editor_property("next_interaction_id", next_id)
    interaction.set_editor_property("target_id", target_id)
    interaction.set_editor_property("objective_text", objective_text)
    interaction.set_editor_property("required", True)
    interaction.set_editor_property("complete_on_start", complete_on_start)
    return interaction


main_level = unreal.load_asset(MAIN_LEVEL_PATH)
singijeon_level = unreal.load_asset(SINGIJEON_LEVEL_PATH)
if not main_level or not singijeon_level:
    raise RuntimeError("L_Main or LV_Singijeon is missing")

main_stage = unreal.ScenarioStageDefinition()
main_stage.set_editor_property("stage_id", "MAIN_SCENE")
main_stage.set_editor_property("stage_name", "Main")
main_stage.set_editor_property("start_interaction_id", "MAIN_INTRO")
main_stage.set_editor_property("interactions", [
    make_interaction(
        "MAIN_INTRO", unreal.ScenarioInteractionType.OBJECTIVE,
        next_id="MAIN_TRAVEL_SINGIJEON",
        objective_text="신기전 체험 구역으로 이동하세요.",
        complete_on_start=True,
    ),
    make_interaction(
        "MAIN_TRAVEL_SINGIJEON", unreal.ScenarioInteractionType.CUSTOM,
        target_id="Travel_Singijeon",
    ),
    make_interaction(
        "MAIN_RETURNED", unreal.ScenarioInteractionType.OBJECTIVE,
        objective_text="신기전 체험 완료. Main 진행이 복원되었습니다.",
    ),
])

main_scenario = create_or_load_data_asset(
    MAIN_SCENARIO_PATH, "/Script/SuwonSiegeContestVR.ScenarioDefinition"
)
main_scenario.set_editor_property("scenario_id", "SCENARIO_Main")
main_scenario.set_editor_property("scenario_name", "Main")
main_scenario.set_editor_property("stages", [main_stage])
main_scenario.set_editor_property("start_stage_id", "MAIN_SCENE")
main_scenario.set_editor_property("narration_table", None)
unreal.EditorAssetLibrary.save_loaded_asset(main_scenario, only_if_is_dirty=False)

main_experience = create_or_load_data_asset(
    MAIN_EXPERIENCE_PATH, "/Script/SuwonSiegeContestVR.ExperienceDefinition"
)
main_experience.set_editor_property("experience_id", "EXP_Main")
main_experience.set_editor_property("display_name", "Main")
main_experience.set_editor_property("scenario_definition", main_scenario)
main_experience.set_editor_property("auto_start_scenario", True)
main_experience.set_editor_property("complete_on_scenario_finished", False)
main_experience.set_editor_property("experience_level", main_level)
main_experience.set_editor_property("return_level", None)
main_experience.set_editor_property("return_on_completion", False)
unreal.EditorAssetLibrary.save_loaded_asset(main_experience, only_if_is_dirty=False)

singijeon_experience = unreal.load_asset(SINGIJEON_EXPERIENCE_PATH)
if not singijeon_experience:
    raise RuntimeError("DA_Experience_Singijeon is missing")
singijeon_experience.set_editor_property("return_level", main_level)
singijeon_experience.set_editor_property("return_on_completion", True)
unreal.EditorAssetLibrary.save_loaded_asset(singijeon_experience, only_if_is_dirty=False)

unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
managers = [actor for actor in actors if isinstance(actor, unreal.ScenarioManagerActor)]
if len(managers) != 1:
    raise RuntimeError(f"Expected exactly one ScenarioManagerActor in L_Main, found {len(managers)}")

manager = managers[0]
manager.set_editor_property("experience_definition", main_experience)
manager.set_editor_property("standalone_scenario_definition", None)
manager.set_editor_property("activate_experience_when_opened_directly", True)
manager.set_editor_property("restore_scenario_checkpoint", True)
manager.refresh_resolved_configuration()

triggers = [actor for actor in actors if isinstance(actor, unreal.ExperienceTravelTriggerActor)]
if triggers:
    trigger = triggers[0]
    for duplicate in triggers[1:]:
        actor_subsystem.destroy_actor(duplicate)
else:
    trigger = actor_subsystem.spawn_actor_from_class(
        unreal.ExperienceTravelTriggerActor, unreal.Vector(300.0, 0.0, 120.0)
    )
trigger.set_actor_label("ExperienceTravelTrigger_Singijeon")
trigger.set_actor_location(unreal.Vector(300.0, 0.0, 120.0), False, False)
trigger.set_editor_property("destination_experience", singijeon_experience)
trigger.set_editor_property("return_scenario_id", "SCENARIO_Main")
trigger.set_editor_property("return_scene_id", "MAIN_SCENE")
trigger.set_editor_property("return_interaction_id", "MAIN_RETURNED")
trigger.set_editor_property("trigger_on_pawn_overlap", True)
trigger.set_editor_property("trigger_on_player_view_location", True)
trigger.set_editor_property("trigger_once", True)
trigger.set_editor_property("trigger_extent", unreal.Vector(100.0, 100.0, 120.0))

player_starts = [actor for actor in actors if isinstance(actor, unreal.PlayerStart)]
if not player_starts:
    player_start = actor_subsystem.spawn_actor_from_class(
        unreal.PlayerStart, unreal.Vector(0.0, 0.0, 120.0)
    )
    player_start.set_actor_label("PlayerStart_Main")

game_mode_class = unreal.load_class(
    None, "/Game/XRFramework/Blueprints/BP_XRGameMode.BP_XRGameMode_C"
)
if not game_mode_class:
    raise RuntimeError("BP_XRGameMode class could not be loaded")
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_world_settings().set_editor_property(
    "default_game_mode", game_mode_class
)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(
    save_map_packages=True, save_content_packages=True
)
unreal.log("MAIN_SINGIJEON_ROUND_TRIP CREATE SUCCESS")
