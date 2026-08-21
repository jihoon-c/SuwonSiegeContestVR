import unreal


LEVEL_PATH = "/GF_Gongsimdon/Maps/LV_Gongsimdon"
SCENARIO_PATH = "/GF_Gongsimdon/Data/DA_Scenario_Gongsimdon"


def make_interaction(interaction_id, interaction_type, target_id, next_id="None",
                     complete_on_start=False):
    interaction = unreal.ScenarioInteraction()
    interaction.set_editor_property("interaction_id", interaction_id)
    interaction.set_editor_property("interaction_type", interaction_type)
    interaction.set_editor_property("target_id", target_id)
    interaction.set_editor_property("next_interaction_id", next_id)
    interaction.set_editor_property("complete_on_start", complete_on_start)
    interaction.set_editor_property("required", True)
    return interaction


scenario = unreal.load_asset(SCENARIO_PATH)
if not scenario:
    raise RuntimeError("DA_Scenario_Gongsimdon is missing")

stage = unreal.ScenarioStageDefinition()
stage.set_editor_property("stage_id", "GONGSIMDON_STAGE_01")
stage.set_editor_property("stage_name", "공심돈 야간 경계 - 동작")
stage.set_editor_property("start_interaction_id", "GONG_ACT_SCAN_PERIMETER")
stage.set_editor_property("interactions", [
    make_interaction(
        "GONG_ACT_SCAN_PERIMETER", unreal.ScenarioInteractionType.OBSERVE,
        "OBS_PERIMETER", "GONG_ACT_SOUND_ANIMAL",
    ),
    make_interaction(
        "GONG_ACT_SOUND_ANIMAL", unreal.ScenarioInteractionType.SEQUENCE,
        "CUE_ANIMAL_SOUND", "GONG_ACT_CHECK_ANIMAL", True,
    ),
    make_interaction(
        "GONG_ACT_CHECK_ANIMAL", unreal.ScenarioInteractionType.OBSERVE,
        "OBS_ANIMAL", "GONG_ACT_SOUND_METAL",
    ),
    make_interaction(
        "GONG_ACT_SOUND_METAL", unreal.ScenarioInteractionType.SEQUENCE,
        "CUE_METAL_SOUND", "GONG_ACT_CHECK_METAL", True,
    ),
    make_interaction(
        "GONG_ACT_CHECK_METAL", unreal.ScenarioInteractionType.OBSERVE,
        "OBS_METAL", "GONG_ACT_REVEAL_ENEMY",
    ),
    make_interaction(
        "GONG_ACT_REVEAL_ENEMY", unreal.ScenarioInteractionType.SPAWN,
        "CUE_REVEAL_ENEMY", "GONG_ACT_IDENTIFY_ENEMY", True,
    ),
    make_interaction(
        "GONG_ACT_IDENTIFY_ENEMY", unreal.ScenarioInteractionType.OBSERVE,
        "OBS_ENEMY_GROUP", "GONG_ACT_REPORT_ENEMY",
    ),
    make_interaction(
        "GONG_ACT_REPORT_ENEMY", unreal.ScenarioInteractionType.CUSTOM,
        "REPORT_ENEMY", "GONG_ACT_ENABLE_BEACON",
    ),
    make_interaction(
        "GONG_ACT_ENABLE_BEACON", unreal.ScenarioInteractionType.SEQUENCE,
        "CUE_ENABLE_BEACON", "GONG_ACT_CHECK_BEACON", True,
    ),
    make_interaction(
        "GONG_ACT_CHECK_BEACON", unreal.ScenarioInteractionType.OBSERVE,
        "OBS_BEACON", "GONG_ACT_RETREAT_ENEMY",
    ),
    make_interaction(
        "GONG_ACT_RETREAT_ENEMY", unreal.ScenarioInteractionType.SEQUENCE,
        "CUE_RETREAT_ENEMY", "GONG_ACT_SHOOT_RETREATING", True,
    ),
    make_interaction(
        "GONG_ACT_SHOOT_RETREATING", unreal.ScenarioInteractionType.COMBAT,
        "COMBAT_RETREATING", "GONG_ACT_FINAL_SCAN",
    ),
    make_interaction(
        "GONG_ACT_FINAL_SCAN", unreal.ScenarioInteractionType.OBSERVE,
        "OBS_FINAL_AREA",
    ),
])
scenario.set_editor_property("scenario_id", "SCENARIO_Gongsimdon")
scenario.set_editor_property("scenario_name", "공심돈 야간 경계")
scenario.set_editor_property("stages", [stage])
scenario.set_editor_property("start_stage_id", "GONGSIMDON_STAGE_01")
unreal.EditorAssetLibrary.save_loaded_asset(scenario, only_if_is_dirty=False)

world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
if not world:
    raise RuntimeError("LV_Gongsimdon could not be loaded")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = list(actor_subsystem.get_all_level_actors())


def find_by_label(label):
    return next((actor for actor in actors if actor.get_actor_label() == label), None)


def find_or_spawn(label, actor_class, location):
    actor = find_by_label(label)
    if actor and not isinstance(actor, actor_class):
        raise RuntimeError(f"{label} exists with the wrong class")
    if not actor:
        actor = actor_subsystem.spawn_actor_from_class(
            actor_class, location, unreal.Rotator()
        )
        actor.set_actor_label(label)
        actors.append(actor)
    actor.set_actor_location(location, False, False)
    return actor


player_start = next((actor for actor in actors if isinstance(actor, unreal.PlayerStart)), None)
origin = player_start.get_actor_location() if player_start else unreal.Vector(0.0, 0.0, 120.0)


def offset(x, y, z):
    return unreal.Vector(origin.x + x, origin.y + y, origin.z + z)


find_or_spawn(
    "Gongsimdon_ScenarioDirector", unreal.GongsimdonScenarioDirectorActor, origin
)

observation_specs = [
    ("Gongsimdon_OBS_Perimeter", "OBS_PERIMETER", (900.0, 0.0, 80.0), 2.0, 24.0),
    ("Gongsimdon_OBS_Animal", "OBS_ANIMAL", (1300.0, 550.0, 20.0), 1.5, 18.0),
    ("Gongsimdon_OBS_Metal", "OBS_METAL", (1250.0, -650.0, 30.0), 1.5, 18.0),
    ("Gongsimdon_OBS_Beacon", "OBS_BEACON", (-1600.0, 650.0, 500.0), 2.0, 18.0),
    ("Gongsimdon_OBS_FinalArea", "OBS_FINAL_AREA", (1100.0, 0.0, 100.0), 2.0, 28.0),
]
for label, target_id, position, view_time, view_angle in observation_specs:
    actor = find_or_spawn(
        label, unreal.GongsimdonObservationTargetActor, offset(*position)
    )
    actor.set_editor_property("target_id", target_id)
    actor.set_editor_property("required_view_time", view_time)
    actor.set_editor_property("required_view_angle", view_angle)
    actor.set_editor_property("max_distance", 5000.0)
    actor.set_editor_property("require_line_of_sight", False)

report_actor = find_or_spawn(
    "Gongsimdon_Report", unreal.GongsimdonReportActor, offset(100.0, 0.0, 0.0)
)
report_actor.set_editor_property("target_id", "REPORT_ENEMY")
report_actor.set_editor_property(
    "expected_direction", unreal.GongsimdonReportDirection.EAST
)
report_actor.set_editor_property("minimum_enemy_count", 6)
report_actor.set_editor_property("maximum_enemy_count", 8)

for obsolete_label in (
    "Gongsimdon_OBS_EnemyGroup",
    "Gongsimdon_Combat_Retreating",
):
    obsolete_actor = find_by_label(obsolete_label)
    if obsolete_actor:
        actor_subsystem.destroy_actor(obsolete_actor)
        actors.remove(obsolete_actor)

enemy_group = find_or_spawn(
    "Gongsimdon_EnemyGroup",
    unreal.GongsimdonEnemyGroupActor,
    offset(400.0, -850.0, -90.0),
)
enemy_group.set_editor_property("enemy_count", 7)
enemy_group.set_editor_property("formation_columns", 3)
enemy_group.set_editor_property("lateral_spacing", 140.0)
enemy_group.set_editor_property("row_spacing", 180.0)
enemy_group.set_editor_property("approach_speed", 140.0)
enemy_group.set_editor_property("retreat_speed", 260.0)
enemy_group.set_editor_property("observation_target_id", "OBS_ENEMY_GROUP")
enemy_group.set_editor_property("combat_target_id", "COMBAT_RETREATING")
enemy_group.set_editor_property("required_combat_hits", 1)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("GONGSIMDON_ACTION_INTERACTIONS CONFIGURE SUCCESS")
