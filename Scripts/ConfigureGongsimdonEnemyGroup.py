import unreal


LEVEL_PATH = "/GF_Gongsimdon/Maps/LV_Gongsimdon"
world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
if not world:
    raise RuntimeError("LV_Gongsimdon could not be loaded")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = list(actor_subsystem.get_all_level_actors())
player_start = next((actor for actor in actors if isinstance(actor, unreal.PlayerStart)), None)
origin = player_start.get_actor_location() if player_start else unreal.Vector(0.0, 0.0, 120.0)

for actor in list(actors):
    if actor.get_actor_label() in (
        "Gongsimdon_OBS_EnemyGroup",
        "Gongsimdon_Combat_Retreating",
    ):
        actor_subsystem.destroy_actor(actor)
        actors.remove(actor)

enemy_group = next((actor for actor in actors
                    if isinstance(actor, unreal.GongsimdonEnemyGroupActor)), None)
if not enemy_group:
    enemy_group = actor_subsystem.spawn_actor_from_class(
        unreal.GongsimdonEnemyGroupActor,
        unreal.Vector(origin.x + 400.0, origin.y - 850.0, origin.z - 90.0),
        unreal.Rotator(),
    )
enemy_group.set_actor_label("Gongsimdon_EnemyGroup")
enemy_group.set_actor_location(
    unreal.Vector(origin.x + 400.0, origin.y - 850.0, origin.z - 90.0),
    False,
    False,
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
unreal.log("GONGSIMDON_ENEMY_GROUP CONFIGURE SUCCESS")
