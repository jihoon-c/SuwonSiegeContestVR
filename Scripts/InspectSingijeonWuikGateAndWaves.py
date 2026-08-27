import unreal


LEVEL_PATH = "/Game/Maps/LV_Singijeon"

world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
if not world:
    raise RuntimeError("LV_Singijeon could not be loaded")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = list(actor_subsystem.get_all_level_actors())
waves = [actor for actor in actors
         if isinstance(actor, unreal.SingijeonEnemyWaveActor)]
gates = [actor for actor in actors
         if "Wuik_Gate" in actor.get_actor_label()
         or "Wuik_Gate" in actor.get_class().get_name()]

for gate in gates:
    unreal.log(
        f"WUIK_GATE label={gate.get_actor_label()} class={gate.get_class().get_name()} "
        f"location={gate.get_actor_location()} forward={gate.get_actor_forward_vector()} "
        f"bounds={gate.get_actor_bounds(False)}")

for wave in waves:
    destination_actor = wave.get_editor_property("destination_actor")
    destination_point = wave.get_editor_property("default_target_point")
    unreal.log(
        f"ENEMY_WAVE label={wave.get_actor_label()} location={wave.get_actor_location()} "
        f"destination_actor={destination_actor} "
        f"destination_point={destination_point.get_world_location() if destination_point else None} "
        f"march_sound={wave.get_editor_property('march_sound')} "
        f"march_volume={wave.get_editor_property('march_sound_volume')}")

if not gates:
    raise RuntimeError("BP_Wuik_Gate was not found")
if not waves:
    raise RuntimeError("No Singijeon Enemy Wave was found")

unreal.log(f"SINGIJEON_WUIK_INSPECT SUCCESS gates={len(gates)} waves={len(waves)}")
