"""Create the project-owned Main education assets and place its manager.

Run after compiling the SuwonSiegeContestVR Editor target:
UnrealEditor-Cmd.exe SuwonSiegeContestVR.uproject -ExecutePythonScript=... -unattended
"""

import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"
MAIN_SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"
MAIN_EXPERIENCE_PATH = "/Game/Core/Experience/Definitions/DA_Experience_Main"
ONGSEONG_EXPERIENCE_PATH = "/Game/Core/Experience/Definitions/DA_Experience_Ongseong"
ONGSEONG_LEVEL_PATH = "/GF_OngseongCrossbow/Maps/LV_Ongseong"


def create_or_load_data_asset(asset_path, class_path):
    asset = unreal.load_asset(asset_path)
    if asset:
        return asset
    asset_class = unreal.load_class(None, class_path)
    if not asset_class:
        raise RuntimeError(f"Could not load class {class_path}")
    package_path, asset_name = asset_path.rsplit("/", 1)
    unreal.EditorAssetLibrary.make_directory(package_path)
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", asset_class)
    asset = ASSET_TOOLS.create_asset(asset_name, package_path, asset_class, factory)
    if not asset:
        raise RuntimeError(f"Could not create {asset_path}")
    return asset


main_level = unreal.load_asset(MAIN_LEVEL_PATH)
if not main_level:
    raise RuntimeError(f"Missing Main level: {MAIN_LEVEL_PATH}")

scenario = create_or_load_data_asset(
    MAIN_SCENARIO_PATH,
    "/Script/SuwonSiegeContestVR.MainEducationScenarioDefinition",
)
unreal.EditorAssetLibrary.save_loaded_asset(scenario, only_if_is_dirty=False)

main_experience = create_or_load_data_asset(
    MAIN_EXPERIENCE_PATH,
    "/Script/SuwonSiegeContestVR.ExperienceDefinition",
)
main_experience.set_editor_property("experience_id", "EXP_Main")
main_experience.set_editor_property("display_name", "수원화성 교육")
main_experience.set_editor_property("scenario_definition", scenario)
main_experience.set_editor_property("auto_start_scenario", True)
main_experience.set_editor_property("complete_on_scenario_finished", False)
main_experience.set_editor_property("experience_level", main_level)
main_experience.set_editor_property("return_level", None)
main_experience.set_editor_property("return_on_completion", False)
unreal.EditorAssetLibrary.save_loaded_asset(main_experience, only_if_is_dirty=False)

# Ongseong already has a level but no project-owned Experience Definition. Its feature
# manager must call CompleteCurrentExperience when its playable completion condition is ready.
ongseong_level = unreal.load_asset(ONGSEONG_LEVEL_PATH)
if ongseong_level:
    ongseong_experience = create_or_load_data_asset(
        ONGSEONG_EXPERIENCE_PATH,
        "/Script/SuwonSiegeContestVR.ExperienceDefinition",
    )
    ongseong_experience.set_editor_property("experience_id", "EXP_Ongseong")
    ongseong_experience.set_editor_property("display_name", "옹성")
    ongseong_experience.set_editor_property("scenario_definition", None)
    ongseong_experience.set_editor_property("auto_start_scenario", False)
    ongseong_experience.set_editor_property("complete_on_scenario_finished", False)
    ongseong_experience.set_editor_property("experience_level", ongseong_level)
    ongseong_experience.set_editor_property("return_level", main_level)
    ongseong_experience.set_editor_property("return_on_completion", True)
    unreal.EditorAssetLibrary.save_loaded_asset(ongseong_experience, only_if_is_dirty=False)
else:
    unreal.log_warning(f"Ongseong level is missing; route remains unavailable: {ONGSEONG_LEVEL_PATH}")

unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
education_managers = [
    actor for actor in actors
    if isinstance(actor, unreal.MainEducationScenarioManagerActor)
]

if education_managers:
    manager = education_managers[0]
    for duplicate in education_managers[1:]:
        actor_subsystem.destroy_actor(duplicate)
else:
    legacy_managers = [
        actor for actor in actors
        if isinstance(actor, unreal.ScenarioManagerActor)
    ]
    transform = legacy_managers[0].get_actor_transform() if legacy_managers else unreal.Transform()
    manager = actor_subsystem.spawn_actor_from_class(
        unreal.MainEducationScenarioManagerActor,
        transform.translation,
        transform.rotation.rotator(),
    )
    for legacy_manager in legacy_managers:
        actor_subsystem.destroy_actor(legacy_manager)

manager.set_actor_label("BP_MainEducationScenarioManager")
manager.set_editor_property("experience_definition", main_experience)
manager.set_editor_property("standalone_scenario_definition", None)
manager.set_editor_property("level_narration_table", None)
manager.set_editor_property("activate_experience_when_opened_directly", True)
manager.set_editor_property("restore_scenario_checkpoint", True)
manager.set_editor_property("mirror_text_to_vrhud", True)
manager.refresh_resolved_configuration()

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log(
    "MAIN_EDUCATION_FLOW SUCCESS: scenario, Main experience, Ongseong route, and Main manager configured"
)
