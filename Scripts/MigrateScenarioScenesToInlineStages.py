import unreal


CONFIGS = [
    {
        "scenario": "/Game/Data/DA_Scenario_Main",
        "legacy_scene": "/Game/Data/DA_Main",
        "experience": "/Game/Core/Experience/Definitions/DA_Experience_Main",
        "level": "/Game/Maps/Main/L_Main",
        "narration": None,
        "complete_on_finish": False,
    },
    {
        "scenario": "/Game/Data/DA_Scenario_Singijeon",
        "legacy_scene": "/Game/Data/DA_Scene_Singijeon",
        "experience": "/Game/Core/Experience/Definitions/DA_Experience_Singijeon",
        "level": "/Game/Maps/LV_Singijeon",
        "narration": "/Game/Data/DT_Narration",
        "complete_on_finish": True,
    },
]


def migrate(config):
    scenario = unreal.load_asset(config["scenario"])
    legacy_scene = unreal.load_asset(config["legacy_scene"])
    experience = unreal.load_asset(config["experience"])
    narration = unreal.load_asset(config["narration"]) if config["narration"] else None
    if not scenario or not experience:
        raise RuntimeError(f"Missing migration input for {config['scenario']}")

    if legacy_scene:
        stage = unreal.ScenarioStageDefinition()
        stage.set_editor_property("stage_id", legacy_scene.get_editor_property("scene_id"))
        stage.set_editor_property("stage_name", legacy_scene.get_editor_property("scene_name"))
        stage.set_editor_property(
            "start_interaction_id", legacy_scene.get_editor_property("start_interaction_id")
        )
        stage.set_editor_property(
            "next_stage_id", legacy_scene.get_editor_property("next_scene_id")
        )
        stage.set_editor_property(
            "interactions", list(legacy_scene.get_editor_property("interactions"))
        )
    else:
        existing_stages = list(scenario.get_editor_property("stages"))
        if len(existing_stages) != 1:
            raise RuntimeError(f"No legacy Scene or existing Stage for {config['scenario']}")
        stage = existing_stages[0]

    scenario.set_editor_property("stages", [stage])
    scenario.set_editor_property("start_stage_id", stage.get_editor_property("stage_id"))
    scenario.set_editor_property("narration_table", narration)
    unreal.EditorAssetLibrary.save_loaded_asset(scenario, only_if_is_dirty=False)

    experience.set_editor_property("scenario_definition", scenario)
    experience.set_editor_property("auto_start_scenario", True)
    experience.set_editor_property(
        "complete_on_scenario_finished", config["complete_on_finish"]
    )
    unreal.EditorAssetLibrary.save_loaded_asset(experience, only_if_is_dirty=False)

    unreal.EditorLoadingAndSavingUtils.load_map(config["level"])
    managers = [
        actor
        for actor in unreal.get_editor_subsystem(
            unreal.EditorActorSubsystem
        ).get_all_level_actors()
        if isinstance(actor, unreal.ScenarioManagerActor)
    ]
    if len(managers) != 1:
        raise RuntimeError(
            f"Expected one ScenarioManagerActor in {config['level']}, found {len(managers)}"
        )
    manager = managers[0]
    manager.set_editor_property("experience_definition", experience)
    manager.set_editor_property("standalone_scenario_definition", None)
    manager.refresh_resolved_configuration()
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(
        save_map_packages=True, save_content_packages=True
    )


for item in CONFIGS:
    migrate(item)

unreal.log("INLINE_SCENARIO_STAGE MIGRATION SUCCESS")
