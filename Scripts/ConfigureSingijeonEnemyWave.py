import unreal


LEVEL_PATH = "/Game/Maps/LV_Singijeon"
PROXY_MESH_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/SKM_Low_Poly_Samurai_VR"
PROXY_PROVIDER_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/DA_SingijeonSamuraiRifleRun_GPU"
RUN_ANIMATION_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/MF_Rifle_Jog_Fwd_Samurai"

world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
if not world:
    raise RuntimeError("LV_Singijeon could not be loaded")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = list(actor_subsystem.get_all_level_actors())
hwacha = next((actor for actor in actors
               if isinstance(actor, unreal.SingijeonHwachaActor)), None)
if not hwacha:
    raise RuntimeError("LV_Singijeon has no Singijeon Hwacha")

waves = [actor for actor in actors
         if isinstance(actor, unreal.SingijeonEnemyWaveActor)]
wave = waves[0] if waves else actor_subsystem.spawn_actor_from_class(
    unreal.SingijeonEnemyWaveActor,
    hwacha.get_actor_location() + hwacha.get_actor_forward_vector() * 9000.0,
    unreal.Rotator(),
)
for duplicate in waves[1:]:
    actor_subsystem.destroy_actor(duplicate)

spawn_location = hwacha.get_actor_location() + hwacha.get_actor_forward_vector() * 9000.0
wave.set_actor_label("Singijeon_EnemyWave")
wave.set_actor_location(spawn_location, False, False)
wave.set_editor_property("target_actor", hwacha)
wave.set_editor_property("enemy_count", 45)
wave.set_editor_property("max_interactive_enemies", 3)
wave.set_editor_property("platoon_count", 3)
wave.set_editor_property("formation_columns_per_platoon", 5)
wave.set_editor_property("charge_speed", 220.0)
wave.set_editor_property("proxy_update_interval", 0.1)
wave.set_editor_property("proxy_start_cull_distance", 5000)
wave.set_editor_property("proxy_end_cull_distance", 12000)
wave.set_editor_property("proxy_skeletal_mesh", unreal.load_asset(PROXY_MESH_PATH))
wave.set_editor_property("proxy_animation_provider", unreal.load_asset(PROXY_PROVIDER_PATH))
wave.set_editor_property("foreground_run_animation", unreal.load_asset(RUN_ANIMATION_PATH))
wave.set_editor_property("proxy_animation_min_screen_size", 0.006)
wave.set_editor_property("proxy_min_lod", 1)
wave.set_editor_property("min_run_animation_rate", 0.86)
wave.set_editor_property("max_run_animation_rate", 1.14)
wave.set_editor_property("formation_random_seed", 741953)
wave.set_editor_property("lateral_jitter", 52.0)
wave.set_editor_property("longitudinal_jitter", 68.0)
wave.set_editor_property("yaw_jitter_degrees", 11.0)
wave.set_editor_property("scale_variation", 0.055)
wave.set_editor_property("auto_find_hwacha", True)
wave.set_editor_property("start_when_hwacha_loaded", True)
wave.set_editor_property("start_on_begin_play", False)
wave.set_editor_property("limit_approach_by_hwacha_procedure", True)
wave.set_editor_property("loaded_approach_limit", 0.35)
wave.set_editor_property("aimed_approach_limit", 0.60)
wave.set_editor_property("igniting_approach_limit", 0.82)
wave.set_editor_property("firing_approach_limit", 0.95)
wave.set_editor_property("volley_casualty_fraction", 1.0)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("SINGIJEON_ENEMY_WAVE CONFIGURE SUCCESS")

