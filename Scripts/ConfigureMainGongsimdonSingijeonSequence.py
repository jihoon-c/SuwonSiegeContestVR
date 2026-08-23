import sys
from pathlib import Path

import unreal


sys.path.insert(0, str(Path(__file__).resolve().parent))
from GongsimdonNarratedScenarioData import (  # noqa: E402
    create_or_update_narration_table,
    make_gongsimdon_stage,
)


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"
GONGSIMDON_LEVEL_PATH = "/GF_Gongsimdon/Maps/LV_Gongsimdon"
SINGIJEON_LEVEL_PATH = "/Game/Maps/LV_Singijeon"
MAIN_SCENARIO_PATH = "/Game/Data/DA_Scenario_Main"
GONGSIMDON_SCENARIO_PATH = "/GF_Gongsimdon/Data/DA_Scenario_Gongsimdon"
MAIN_EXPERIENCE_PATH = "/Game/Core/Experience/Definitions/DA_Experience_Main"
GONGSIMDON_EXPERIENCE_PATH = "/Game/Core/Experience/Definitions/DA_Experience_Gongsimdon"
SINGIJEON_EXPERIENCE_PATH = "/Game/Core/Experience/Definitions/DA_Experience_Singijeon"


def create_or_load_data_asset(asset_path, class_path):
    asset = unreal.load_asset(asset_path)
    if asset:
        return asset
    asset_class = unreal.load_class(None, class_path)
    if not asset_class:
        raise RuntimeError(f"Could not load {class_path}")
    package_path, asset_name = asset_path.rsplit("/", 1)
    unreal.EditorAssetLibrary.make_directory(package_path)
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", asset_class)
    asset = ASSET_TOOLS.create_asset(asset_name, package_path, asset_class, factory)
    if not asset:
        raise RuntimeError(f"Could not create {asset_path}")
    return asset


def make_interaction(interaction_id, interaction_type, next_id="None",
                     target_id="None", objective_text="", complete_on_start=False,
                     duration=0.0):
    interaction = unreal.ScenarioInteraction()
    interaction.set_editor_property("interaction_id", interaction_id)
    interaction.set_editor_property("interaction_type", interaction_type)
    interaction.set_editor_property("next_interaction_id", next_id)
    interaction.set_editor_property("target_id", target_id)
    interaction.set_editor_property("objective_text", objective_text)
    interaction.set_editor_property("complete_on_start", complete_on_start)
    interaction.set_editor_property("duration", duration)
    interaction.set_editor_property("required", True)
    return interaction


main_level = unreal.load_asset(MAIN_LEVEL_PATH)
gongsimdon_level = unreal.load_asset(GONGSIMDON_LEVEL_PATH)
singijeon_level = unreal.load_asset(SINGIJEON_LEVEL_PATH)
if not main_level or not gongsimdon_level or not singijeon_level:
    raise RuntimeError("Main, Gongsimdon, or Singijeon Level is missing")

main_stage = unreal.ScenarioStageDefinition()
main_stage.set_editor_property("stage_id", "MAIN_SCENE")
main_stage.set_editor_property("stage_name", "Main")
main_stage.set_editor_property("start_interaction_id", "MAIN_INTRO")
main_stage.set_editor_property("interactions", [
    make_interaction(
        "MAIN_INTRO", unreal.ScenarioInteractionType.OBJECTIVE,
        next_id="MAIN_TRAVEL_GONGSIMDON",
        objective_text="먼저 공심돈 체험 구역으로 이동하세요.",
        complete_on_start=True,
    ),
    make_interaction(
        "MAIN_TRAVEL_GONGSIMDON", unreal.ScenarioInteractionType.CUSTOM,
        next_id="MAIN_AFTER_GONGSIMDON", target_id="Travel_Gongsimdon",
    ),
    make_interaction(
        "MAIN_AFTER_GONGSIMDON", unreal.ScenarioInteractionType.OBJECTIVE,
        next_id="MAIN_TRAVEL_SINGIJEON",
        objective_text="공심돈 1번 시나리오 완료. 다음은 신기전 체험입니다.",
        complete_on_start=True,
    ),
    make_interaction(
        "MAIN_TRAVEL_SINGIJEON", unreal.ScenarioInteractionType.CUSTOM,
        next_id="MAIN_AFTER_SINGIJEON", target_id="Travel_Singijeon",
    ),
    make_interaction(
        "MAIN_AFTER_SINGIJEON", unreal.ScenarioInteractionType.OBJECTIVE,
        objective_text="신기전 체험 완료. Main 진행이 복원되었습니다.",
        complete_on_start=False,
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

gongsimdon_stage = unreal.ScenarioStageDefinition()
gongsimdon_stage.set_editor_property("stage_id", "GONGSIMDON_STAGE_01")
gongsimdon_stage.set_editor_property("stage_name", "공심돈 야간 경계 - 동작")
gongsimdon_stage.set_editor_property("start_interaction_id", "GONG_ACT_SCAN_PERIMETER")
gongsimdon_stage.set_editor_property("interactions", [
    make_interaction(
        "GONG_ACT_SCAN_PERIMETER", unreal.ScenarioInteractionType.OBSERVE,
        next_id="GONG_ACT_SOUND_ANIMAL", target_id="OBS_PERIMETER",
    ),
    make_interaction(
        "GONG_ACT_SOUND_ANIMAL", unreal.ScenarioInteractionType.SEQUENCE,
        next_id="GONG_ACT_CHECK_ANIMAL", target_id="CUE_ANIMAL_SOUND",
        complete_on_start=True,
    ),
    make_interaction(
        "GONG_ACT_CHECK_ANIMAL", unreal.ScenarioInteractionType.OBSERVE,
        next_id="GONG_ACT_SOUND_METAL", target_id="OBS_ANIMAL",
    ),
    make_interaction(
        "GONG_ACT_SOUND_METAL", unreal.ScenarioInteractionType.SEQUENCE,
        next_id="GONG_ACT_CHECK_METAL", target_id="CUE_METAL_SOUND",
        complete_on_start=True,
    ),
    make_interaction(
        "GONG_ACT_CHECK_METAL", unreal.ScenarioInteractionType.OBSERVE,
        next_id="GONG_ACT_REVEAL_ENEMY", target_id="OBS_METAL",
    ),
    make_interaction(
        "GONG_ACT_REVEAL_ENEMY", unreal.ScenarioInteractionType.SPAWN,
        next_id="GONG_ACT_IDENTIFY_ENEMY", target_id="CUE_REVEAL_ENEMY",
        complete_on_start=True,
    ),
    make_interaction(
        "GONG_ACT_IDENTIFY_ENEMY", unreal.ScenarioInteractionType.OBSERVE,
        next_id="GONG_ACT_REPORT_ENEMY", target_id="OBS_ENEMY_GROUP",
    ),
    make_interaction(
        "GONG_ACT_REPORT_ENEMY", unreal.ScenarioInteractionType.CUSTOM,
        next_id="GONG_ACT_ENABLE_BEACON", target_id="REPORT_ENEMY",
    ),
    make_interaction(
        "GONG_ACT_ENABLE_BEACON", unreal.ScenarioInteractionType.SEQUENCE,
        next_id="GONG_ACT_CHECK_BEACON", target_id="CUE_ENABLE_BEACON",
        complete_on_start=True,
    ),
    make_interaction(
        "GONG_ACT_CHECK_BEACON", unreal.ScenarioInteractionType.OBSERVE,
        next_id="GONG_ACT_RETREAT_ENEMY", target_id="OBS_BEACON",
    ),
    make_interaction(
        "GONG_ACT_RETREAT_ENEMY", unreal.ScenarioInteractionType.SEQUENCE,
        next_id="GONG_ACT_SHOOT_RETREATING", target_id="CUE_RETREAT_ENEMY",
        complete_on_start=True,
    ),
    make_interaction(
        "GONG_ACT_SHOOT_RETREATING", unreal.ScenarioInteractionType.COMBAT,
        next_id="GONG_ACT_FINAL_SCAN", target_id="COMBAT_RETREATING",
    ),
    make_interaction(
        "GONG_ACT_FINAL_SCAN", unreal.ScenarioInteractionType.OBSERVE,
        target_id="OBS_FINAL_AREA",
    ),
])
gongsimdon_narration_table = create_or_update_narration_table(unreal)
gongsimdon_stage = make_gongsimdon_stage(unreal)
gongsimdon_scenario = create_or_load_data_asset(
    GONGSIMDON_SCENARIO_PATH, "/Script/SuwonSiegeContestVR.ScenarioDefinition"
)
gongsimdon_scenario.set_editor_property("scenario_id", "SCENARIO_Gongsimdon")
gongsimdon_scenario.set_editor_property("scenario_name", "공심돈 야간 경계")
gongsimdon_scenario.set_editor_property("stages", [gongsimdon_stage])
gongsimdon_scenario.set_editor_property("start_stage_id", "GONGSIMDON_STAGE_01")
gongsimdon_scenario.set_editor_property("narration_table", gongsimdon_narration_table)
unreal.EditorAssetLibrary.save_loaded_asset(gongsimdon_scenario, only_if_is_dirty=False)

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

gongsimdon_experience = create_or_load_data_asset(
    GONGSIMDON_EXPERIENCE_PATH, "/Script/SuwonSiegeContestVR.ExperienceDefinition"
)
gongsimdon_experience.set_editor_property("experience_id", "EXP_Gongsimdon")
gongsimdon_experience.set_editor_property("display_name", "공심돈")
gongsimdon_experience.set_editor_property("scenario_definition", gongsimdon_scenario)
gongsimdon_experience.set_editor_property("auto_start_scenario", True)
gongsimdon_experience.set_editor_property("complete_on_scenario_finished", True)
gongsimdon_experience.set_editor_property("experience_level", gongsimdon_level)
gongsimdon_experience.set_editor_property("return_level", main_level)
gongsimdon_experience.set_editor_property("return_on_completion", True)
unreal.EditorAssetLibrary.save_loaded_asset(gongsimdon_experience, only_if_is_dirty=False)

singijeon_experience = unreal.load_asset(SINGIJEON_EXPERIENCE_PATH)
if not singijeon_experience:
    raise RuntimeError("DA_Experience_Singijeon is missing")
singijeon_experience.set_editor_property("return_level", main_level)
singijeon_experience.set_editor_property("return_on_completion", True)
unreal.EditorAssetLibrary.save_loaded_asset(singijeon_experience, only_if_is_dirty=False)


def configure_manager_and_player_start(level_path, experience, player_location):
    unreal.EditorLoadingAndSavingUtils.load_map(level_path)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = actor_subsystem.get_all_level_actors()
    managers = [actor for actor in actors if isinstance(actor, unreal.ScenarioManagerActor)]
    if len(managers) > 1:
        raise RuntimeError(f"{level_path} contains multiple Scenario Managers")
    manager = managers[0] if managers else actor_subsystem.spawn_actor_from_class(
        unreal.ScenarioManagerActor, unreal.Vector(), unreal.Rotator()
    )
    manager.set_actor_label("BP_ScenarioManager")
    manager.set_editor_property("experience_definition", experience)
    manager.set_editor_property("standalone_scenario_definition", None)
    manager.set_editor_property("level_narration_table", None)
    manager.set_editor_property("activate_experience_when_opened_directly", True)
    manager.set_editor_property("restore_scenario_checkpoint", True)
    manager.refresh_resolved_configuration()

    player_starts = [actor for actor in actors if isinstance(actor, unreal.PlayerStart)]
    if not player_starts:
        player_start = actor_subsystem.spawn_actor_from_class(
            unreal.PlayerStart, player_location, unreal.Rotator()
        )
        player_start.set_actor_label("PlayerStart_Gongsimdon")

    game_mode_class = unreal.load_class(
        None, "/Game/XRFramework/Blueprints/BP_XRGameMode.BP_XRGameMode_C"
    )
    unreal.get_editor_subsystem(
        unreal.UnrealEditorSubsystem
    ).get_editor_world().get_world_settings().set_editor_property(
        "default_game_mode", game_mode_class
    )
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)


configure_manager_and_player_start(
    GONGSIMDON_LEVEL_PATH, gongsimdon_experience, unreal.Vector(0.0, 0.0, 120.0)
)

unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
managers = [actor for actor in actors if isinstance(actor, unreal.ScenarioManagerActor)]
if len(managers) != 1:
    raise RuntimeError(f"Expected one Main Scenario Manager, found {len(managers)}")
main_manager = managers[0]
main_manager.set_editor_property("experience_definition", main_experience)
main_manager.set_editor_property("standalone_scenario_definition", None)
main_manager.set_editor_property("activate_experience_when_opened_directly", True)
main_manager.set_editor_property("restore_scenario_checkpoint", True)
main_manager.refresh_resolved_configuration()


def find_or_spawn_trigger(label, location):
    actor = next((item for item in actors if item.get_actor_label() == label), None)
    if not actor:
        actor = actor_subsystem.spawn_actor_from_class(
            unreal.ExperienceTravelTriggerActor, location, unreal.Rotator()
        )
        actor.set_actor_label(label)
        actors.append(actor)
    actor.set_actor_location(location, False, False)
    actor.set_editor_property("return_scenario_id", "SCENARIO_Main")
    actor.set_editor_property("return_scene_id", "MAIN_SCENE")
    actor.set_editor_property("trigger_on_pawn_overlap", True)
    actor.set_editor_property("trigger_on_player_view_location", True)
    actor.set_editor_property("trigger_once", True)
    actor.set_editor_property("trigger_extent", unreal.Vector(100.0, 100.0, 120.0))
    return actor


gongsimdon_trigger = find_or_spawn_trigger(
    "ExperienceTravelTrigger_Gongsimdon", unreal.Vector(300.0, -220.0, 120.0)
)
gongsimdon_trigger.set_editor_property("destination_experience", gongsimdon_experience)
gongsimdon_trigger.set_editor_property("required_interaction_id", "MAIN_TRAVEL_GONGSIMDON")
gongsimdon_trigger.set_editor_property("return_interaction_id", "MAIN_AFTER_GONGSIMDON")

singijeon_trigger = find_or_spawn_trigger(
    "ExperienceTravelTrigger_Singijeon", unreal.Vector(600.0, 220.0, 120.0)
)
singijeon_trigger.set_editor_property("destination_experience", singijeon_experience)
singijeon_trigger.set_editor_property("required_interaction_id", "MAIN_TRAVEL_SINGIJEON")
singijeon_trigger.set_editor_property("return_interaction_id", "MAIN_AFTER_SINGIJEON")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("MAIN_GONGSIMDON_SINGIJEON_SEQUENCE CONFIGURE SUCCESS")
