import unreal


LEVEL_PATH = "/Game/Maps/LV_Singijeon"
SIDE_WAVE_LABELS = {"Singijeon_EnemyWave_Left", "Singijeon_EnemyWave_Right"}
PROXY_MESH_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/SKM_Low_Poly_Samurai_VR"
RUN_ANIMATION_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/MF_Rifle_Jog_Fwd_Samurai"

world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
if not world:
    raise RuntimeError("LV_Singijeon could not be loaded")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors_before = list(actor_subsystem.get_all_level_actors())
transforms_before = {
    actor.get_path_name(): actor.get_actor_transform()
    for actor in actors_before
}

for actor in actors_before:
    if (isinstance(actor, unreal.SingijeonEnemyWaveActor) and
            actor.get_actor_label() in SIDE_WAVE_LABELS):
        actor_subsystem.destroy_actor(actor)

actors_after_delete = list(actor_subsystem.get_all_level_actors())
waves = [actor for actor in actors_after_delete
         if isinstance(actor, unreal.SingijeonEnemyWaveActor)]
if len(waves) != 1:
    labels = ", ".join(wave.get_actor_label() for wave in waves)
    raise RuntimeError(f"Expected one original Enemy Wave, found {len(waves)}: {labels}")

wave = waves[0]
central_transform = wave.get_actor_transform()
hwacha = next((actor for actor in actors_after_delete
               if isinstance(actor, unreal.SingijeonHwachaActor)), None)
if not hwacha:
    raise RuntimeError("LV_Singijeon has no Singijeon Hwacha")

wave.set_editor_property("target_actor", hwacha)
wave.set_editor_property("enemy_count", 45)
wave.set_editor_property("max_interactive_enemies", 3)
wave.set_editor_property("proxy_update_interval", 0.1)
wave.set_editor_property("proxy_skeletal_mesh", unreal.load_asset(PROXY_MESH_PATH))
wave.set_editor_property("foreground_run_animation", unreal.load_asset(RUN_ANIMATION_PATH))
wave.set_editor_property("proxy_scale", unreal.Vector(0.9, 0.9, 0.9))
wave.set_editor_property("desired_enemy_height", 175.0)
wave.set_editor_property("use_gpu_instanced_crowd", False)
wave.set_editor_property("shared_pose_leader_count", 8)
wave.set_editor_property("show_enemies_while_ready", True)
wave.set_editor_property("volley_casualty_fraction", 1.0)

if not wave.get_actor_transform().equals(central_transform):
    raise RuntimeError("Original Enemy Wave transform changed unexpectedly")

for actor in actor_subsystem.get_all_level_actors():
    path = actor.get_path_name()
    if path in transforms_before and not actor.get_actor_transform().equals(transforms_before[path]):
        raise RuntimeError(f"Existing actor transform changed unexpectedly: {path}")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("SINGIJEON_ENEMY_WAVE RETARGET-FIX CHECKPOINT ROLLBACK SUCCESS")
