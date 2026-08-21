import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"GONGSIMDON_ENEMY_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"GONGSIMDON_ENEMY_VERIFY FAIL: {message}")


soldier_class = unreal.load_class(
    None, "/Script/SuwonSiegeContestVR.EnemySoldierActor"
)
check(soldier_class is not None, "Shared Enemy Soldier class loads")

unreal.EditorLoadingAndSavingUtils.load_map("/GF_Gongsimdon/Maps/LV_Gongsimdon")
actors = list(unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem
).get_all_level_actors())
groups = [actor for actor in actors if isinstance(actor, unreal.GongsimdonEnemyGroupActor)]
check(len(groups) == 1, "Level contains exactly one Enemy Group")
check(not any(actor.get_actor_label() == "Gongsimdon_OBS_EnemyGroup" for actor in actors),
      "Old static enemy observation placeholder is removed")
check(not any(actor.get_actor_label() == "Gongsimdon_Combat_Retreating" for actor in actors),
      "Old static combat placeholder is removed")
if groups:
    group = groups[0]
    check(group.get_editor_property("enemy_count") == 7,
          "Enemy Group spawns seven soldiers")
    check(group.get_editor_property("formation_columns") == 3,
          "Enemy Group uses three formation columns")
    check(group.get_editor_property("enemy_class") is not None,
          "Enemy Group references a Shared Enemy Soldier class")
    check(str(group.get_editor_property("observation_target_id")) ==
          "OBS_ENEMY_GROUP", "Group handles enemy observation")
    check(str(group.get_editor_property("combat_target_id")) ==
          "COMBAT_RETREATING", "Group handles retreat combat")

if errors:
    raise RuntimeError("Gongsimdon enemy verification failed: " + "; ".join(errors))
unreal.log("GONGSIMDON_ENEMY_GROUP VERIFY SUCCESS")
