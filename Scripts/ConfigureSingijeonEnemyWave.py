import unreal


LEVEL_PATH = "/Game/Maps/LV_Singijeon"

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
wave.set_editor_property("max_interactive_enemies", 10)
wave.set_editor_property("platoon_count", 3)
wave.set_editor_property("formation_columns_per_platoon", 5)
wave.set_editor_property("charge_speed", 220.0)
wave.set_editor_property("proxy_update_interval", 0.0667)
wave.set_editor_property("auto_find_hwacha", True)
wave.set_editor_property("start_when_hwacha_loaded", True)
wave.set_editor_property("start_on_begin_play", False)
wave.set_editor_property("volley_casualty_fraction", 1.0)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("SINGIJEON_ENEMY_WAVE CONFIGURE SUCCESS")

