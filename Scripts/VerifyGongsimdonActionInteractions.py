import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"GONGSIMDON_ACTION_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"GONGSIMDON_ACTION_VERIFY FAIL: {message}")


scenario = unreal.load_asset("/GF_Gongsimdon/Data/DA_Scenario_Gongsimdon")
check(scenario is not None, "DA_Scenario_Gongsimdon loads")

expected_ids = [
    "GONG_ACT_SCAN_PERIMETER",
    "GONG_ACT_SOUND_ANIMAL",
    "GONG_ACT_CHECK_ANIMAL",
    "GONG_ACT_SOUND_METAL",
    "GONG_ACT_CHECK_METAL",
    "GONG_ACT_REVEAL_ENEMY",
    "GONG_ACT_IDENTIFY_ENEMY",
    "GONG_ACT_REPORT_ENEMY",
    "GONG_ACT_ENABLE_BEACON",
    "GONG_ACT_CHECK_BEACON",
    "GONG_ACT_RETREAT_ENEMY",
    "GONG_ACT_SHOOT_RETREATING",
    "GONG_ACT_FINAL_SCAN",
]
if scenario:
    stages = list(scenario.get_editor_property("stages"))
    interactions = list(stages[0].get_editor_property("interactions")) if stages else []
    ids = [str(item.get_editor_property("interaction_id")) for item in interactions]
    check(ids == expected_ids, "Action interactions are ordered as authored")
    check(all(item.get_editor_property("interaction_type") !=
              unreal.ScenarioInteractionType.NARRATION for item in interactions),
          "Scenario contains no Narration interaction")
    check(str(stages[0].get_editor_property("start_interaction_id")) == expected_ids[0],
          "Stage starts with perimeter observation")

unreal.EditorLoadingAndSavingUtils.load_map("/GF_Gongsimdon/Maps/LV_Gongsimdon")
actors = list(unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem
).get_all_level_actors())

directors = [actor for actor in actors
             if isinstance(actor, unreal.GongsimdonScenarioDirectorActor)]
observations = [actor for actor in actors
                if isinstance(actor, unreal.GongsimdonObservationTargetActor)]
reports = [actor for actor in actors if isinstance(actor, unreal.GongsimdonReportActor)]
combat_targets = [actor for actor in actors
                  if isinstance(actor, unreal.GongsimdonCombatTargetActor)]
enemy_groups = [actor for actor in actors
                if isinstance(actor, unreal.GongsimdonEnemyGroupActor)]

check(len(directors) == 1, "Level contains one Gongsimdon Scenario Director")
check(len(observations) == 5, "Level contains five static observation targets")
check(len(reports) == 1, "Level contains one report validator")
check(len(combat_targets) == 0, "Standalone placeholder combat target is removed")
check(len(enemy_groups) == 1, "Level contains one real enemy group")

observation_ids = {str(actor.get_editor_property("target_id")) for actor in observations}
check(observation_ids == {
    "OBS_PERIMETER", "OBS_ANIMAL", "OBS_METAL", "OBS_BEACON",
    "OBS_FINAL_AREA",
}, "Observation Target IDs match the Scenario")
if reports:
    check(str(reports[0].get_editor_property("target_id")) == "REPORT_ENEMY",
          "Report Target ID matches the Scenario")
    check(reports[0].get_editor_property("minimum_enemy_count") == 6,
          "Report minimum enemy count is six")
if enemy_groups:
    check(enemy_groups[0].get_editor_property("enemy_count") == 7,
          "Enemy Group is configured for seven soldiers")
    check(str(enemy_groups[0].get_editor_property("observation_target_id")) ==
          "OBS_ENEMY_GROUP", "Enemy Group owns the observation target")
    check(str(enemy_groups[0].get_editor_property("combat_target_id")) ==
          "COMBAT_RETREATING", "Enemy Group owns the combat target")

if errors:
    raise RuntimeError("Gongsimdon action verification failed: " + "; ".join(errors))
unreal.log("GONGSIMDON_ACTION_INTERACTIONS VERIFY SUCCESS")
