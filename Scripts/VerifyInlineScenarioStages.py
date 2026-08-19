import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"INLINE_STAGE_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"INLINE_STAGE_VERIFY FAIL: {message}")


configs = [
    (
        "/Game/Data/DA_Scenario_Main",
        "/Game/Core/Experience/Definitions/DA_Experience_Main",
        "/Game/Maps/Main/L_Main",
        "MAIN_SCENE",
        3,
        None,
        False,
    ),
    (
        "/Game/Data/DA_Scenario_Singijeon",
        "/Game/Core/Experience/Definitions/DA_Experience_Singijeon",
        "/Game/Maps/LV_Singijeon",
        "Singijeon",
        28,
        "/Game/Data/DT_Narration",
        True,
    ),
]

for scenario_path, experience_path, level_path, stage_id, interaction_count, narration_path, complete_on_finish in configs:
    scenario = unreal.load_asset(scenario_path)
    experience = unreal.load_asset(experience_path)
    narration = unreal.load_asset(narration_path) if narration_path else None
    check(scenario is not None, f"{scenario_path} loads")
    check(experience is not None, f"{experience_path} loads")
    if not scenario or not experience:
        continue

    stages = list(scenario.get_editor_property("stages"))
    check(len(stages) == 1, f"{scenario_path} contains one inline Stage")
    check(str(scenario.get_editor_property("start_stage_id")) == stage_id,
          f"{scenario_path} StartStageID is {stage_id}")
    if stages:
        check(str(stages[0].get_editor_property("stage_id")) == stage_id,
              f"Inline Stage ID is {stage_id}")
        check(len(stages[0].get_editor_property("interactions")) == interaction_count,
              f"Inline Stage exposes all {interaction_count} interactions")
    check(scenario.get_editor_property("narration_table") == narration,
          f"{scenario_path} owns its Narration Table")
    check(experience.get_editor_property("scenario_definition") == scenario,
          f"{experience_path} owns its Scenario Definition")
    check(experience.get_editor_property("auto_start_scenario"),
          f"{experience_path} owns auto-start policy")
    check(experience.get_editor_property("complete_on_scenario_finished") == complete_on_finish,
          f"{experience_path} owns completion policy")

    world = unreal.EditorLoadingAndSavingUtils.load_map(level_path)
    check(world is not None, f"{level_path} loads")
    managers = [
        actor
        for actor in unreal.get_editor_subsystem(
            unreal.EditorActorSubsystem
        ).get_all_level_actors()
        if isinstance(actor, unreal.ScenarioManagerActor)
    ]
    check(len(managers) == 1, f"{level_path} contains one Scenario Manager")
    if managers:
        manager = managers[0]
        check(manager.get_editor_property("experience_definition") == experience,
              f"{level_path} Manager only assigns the Experience")
        check(manager.get_editor_property("standalone_scenario_definition") is None,
              f"{level_path} Manager has no duplicate Scenario assignment")
        check(manager.get_editor_property("scenario_definition") == scenario,
              f"{level_path} Manager resolves Scenario from Experience")
        check(manager.get_editor_property("narration_table") == narration,
              f"{level_path} Manager resolves Narration from Scenario")

check(not unreal.EditorAssetLibrary.does_asset_exist("/Game/Data/DA_Main"),
      "Legacy DA_Main Scene asset was removed")
check(not unreal.EditorAssetLibrary.does_asset_exist("/Game/Data/DA_Scene_Singijeon"),
      "Legacy DA_Scene_Singijeon asset was removed")

if errors:
    raise RuntimeError("Inline Stage verification failed: " + "; ".join(errors))
unreal.log("INLINE_SCENARIO_STAGE VERIFY SUCCESS")
