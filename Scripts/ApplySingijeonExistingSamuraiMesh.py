"""Use the existing Singijeon Samurai mesh on its placed wave without moving it."""

import unreal


LEVEL_PATH = "/Game/Maps/LV_Singijeon"
EXISTING_SAMURAI_MESH_PATH = "/GF_Singijeon/Asset/EnemyMan1/Low_Poly_Samurai"


world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
if not world:
    raise RuntimeError(f"Could not load {LEVEL_PATH}")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
waves = [
    actor for actor in actor_subsystem.get_all_level_actors()
    if isinstance(actor, unreal.SingijeonEnemyWaveActor)
]
if len(waves) != 1:
    raise RuntimeError(f"Expected exactly one placed Singijeon wave, found {len(waves)}")

samurai_mesh = unreal.load_asset(EXISTING_SAMURAI_MESH_PATH)
if not isinstance(samurai_mesh, unreal.SkeletalMesh):
    raise RuntimeError(f"Missing existing Singijeon Samurai mesh: {EXISTING_SAMURAI_MESH_PATH}")

wave = waves[0]
location_before = wave.get_actor_location()
rotation_before = wave.get_actor_rotation()
scale_before = wave.get_actor_scale3d()
wave.set_editor_property("proxy_skeletal_mesh", samurai_mesh)
location_after = wave.get_actor_location()
rotation_after = wave.get_actor_rotation()
scale_after = wave.get_actor_scale3d()
rotation_delta = unreal.MathLibrary.normalized_delta_rotator(rotation_after, rotation_before)
if (unreal.MathLibrary.vector_distance(location_before, location_after) > 0.01 or
        unreal.MathLibrary.vector_distance(scale_before, scale_after) > 0.0001 or
        abs(rotation_delta.pitch) > 0.01 or abs(rotation_delta.yaw) > 0.01 or
        abs(rotation_delta.roll) > 0.01):
    raise RuntimeError("Refusing to save: the placed Singijeon wave transform changed")

if not unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True):
    raise RuntimeError(f"Could not save {LEVEL_PATH}")

unreal.log(
    "SINGIJEON_EXISTING_SAMURAI_APPLY SUCCESS: changed ProxySkeletalMesh only; "
    f"wave_location={location_after} wave_rotation={rotation_after} wave_scale={scale_after}"
)
