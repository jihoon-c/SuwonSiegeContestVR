import unreal


LEVEL_PATH = "/Game/Maps/LV_Singijeon"
MARCH_SOUND_PATH = "/GF_Singijeon/Asset/Sound/Effect/Troop_march_2"
FRONT_CLEARANCE = 500.0
LANE_SPACING = 250.0

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
if not waves:
    raise RuntimeError("No Singijeon Enemy Wave was found")
if len(gates) != 1:
    raise RuntimeError(f"Expected one BP_Wuik_Gate, found {len(gates)}")

gate = gates[0]
march_sound = unreal.load_asset(MARCH_SOUND_PATH)
if not march_sound:
    raise RuntimeError(f"March sound is missing: {MARCH_SOUND_PATH}")

gate_forward = gate.get_actor_forward_vector()
gate_right = gate.get_actor_right_vector()
gate_origin, gate_extent = gate.get_actor_bounds(False)
forward_extent = (
    abs(gate_forward.x) * gate_extent.x
    + abs(gate_forward.y) * gate_extent.y
    + abs(gate_forward.z) * gate_extent.z
)

# Pick the gate side already occupied by the waves, so soldiers stop outside
# the gate instead of crossing through it.
average_wave_location = unreal.Vector()
for wave in waves:
    average_wave_location += wave.get_actor_location()
average_wave_location /= float(len(waves))
front_candidate = gate_origin + gate_forward * (forward_extent + FRONT_CLEARANCE)
back_candidate = gate_origin - gate_forward * (forward_extent + FRONT_CLEARANCE)
base_destination = (
    front_candidate
    if (front_candidate - average_wave_location).length()
       < (back_candidate - average_wave_location).length()
    else back_candidate
)

# Preserve route lanes by assigning lateral destinations in the same order as
# the placed wave origins. No Wave Actor transform is changed.
waves.sort(key=lambda wave: (
    (wave.get_actor_location() - gate_origin).dot(gate_right),
    wave.get_actor_label(),
))
original_transforms = {
    wave.get_path_name(): wave.get_actor_transform() for wave in waves
}
center_index = (len(waves) - 1) * 0.5
for index, wave in enumerate(waves):
    destination_point = wave.get_editor_property("default_target_point")
    if not destination_point:
        raise RuntimeError(f"{wave.get_actor_label()} has no Destination Point")

    lane_offset = (index - center_index) * LANE_SPACING
    destination = base_destination + gate_right * lane_offset
    # Retain the authored enemy ground height rather than moving the Wave Actor.
    destination.z = wave.get_actor_location().z

    wave.modify()
    destination_point.modify()
    wave.set_editor_property("destination_actor", None)
    destination_point.set_world_location(destination, False, False)
    wave.set_editor_property("march_sound", march_sound)
    wave.set_editor_property("march_sound_volume", 0.7)
    wave.rebuild_route()

    if not wave.get_actor_transform().equals(original_transforms[wave.get_path_name()]):
        raise RuntimeError(
            f"Enemy Wave transform changed unexpectedly: {wave.get_actor_label()}")
    unreal.log(
        f"WUIK_ROUTE label={wave.get_actor_label()} "
        f"destination={destination_point.get_world_location()} "
        f"march_sound={wave.get_editor_property('march_sound')}")

if not unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True):
    raise RuntimeError("LV_Singijeon could not be saved")

unreal.log(
    f"SINGIJEON_WUIK_ROUTE CONFIGURE SUCCESS waves={len(waves)} "
    f"gate={gate.get_actor_label()} base_destination={base_destination}")
