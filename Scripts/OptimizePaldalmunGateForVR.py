"""Create a VR LOD variant of Paldalmun Gate and replace its L_Main mesh references safely."""

import unreal


SOURCE_MESH_PATH = (
    "/Game/Art/Model/KCISA-수원화성_팔달문_PaldalmunGate_/StaticMeshes/"
    "KCISA-수원화성_팔달문_PaldalmunGate_"
)
DESTINATION_DIRECTORY = "/Game/Art/Model/VR_Optimized"
DESTINATION_MESH_PATH = f"{DESTINATION_DIRECTORY}/SM_PaldalmunGate_VR"
MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"


def new_reduction(percent_triangles, screen_size):
    settings = unreal.StaticMeshReductionSettings()
    settings.set_editor_property("percent_triangles", percent_triangles)
    settings.set_editor_property("screen_size", screen_size)
    return settings


source_mesh = unreal.load_asset(SOURCE_MESH_PATH)
if not isinstance(source_mesh, unreal.StaticMesh):
    raise RuntimeError(f"Missing source static mesh: {SOURCE_MESH_PATH}")

unreal.EditorAssetLibrary.make_directory(DESTINATION_DIRECTORY)
vr_mesh = unreal.load_asset(DESTINATION_MESH_PATH)
if not vr_mesh:
    vr_mesh = unreal.EditorAssetLibrary.duplicate_asset(SOURCE_MESH_PATH, DESTINATION_MESH_PATH)
if not isinstance(vr_mesh, unreal.StaticMesh):
    raise RuntimeError(f"Could not create VR mesh: {DESTINATION_MESH_PATH}")

# Preserve LOD0 visual fidelity; generate inexpensive LODs for normal VR viewing distances.
# StaticMeshEditor is lazily loaded in commandlets, unlike a normal editor session.
unreal.load_module("StaticMeshEditor")
mesh_editor = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
if not mesh_editor:
    raise RuntimeError("StaticMeshEditorSubsystem could not be loaded")
reduction_options = unreal.StaticMeshReductionOptions()
reduction_options.set_editor_property("auto_compute_lod_screen_size", False)
reduction_options.set_editor_property("reduction_settings", [
    new_reduction(0.50, 0.55),
    new_reduction(0.25, 0.25),
    new_reduction(0.10, 0.10),
])
lod_count = mesh_editor.set_lods(vr_mesh, reduction_options)
if lod_count < 2:
    raise RuntimeError(f"VR LOD generation failed; result LOD count={lod_count}")

# This is a desktop/OpenXR LOD asset; use conventional LODs rather than enabling Nanite.
nanite_settings = vr_mesh.get_editor_property("nanite_settings")
nanite_settings.set_editor_property("enabled", False)
vr_mesh.set_editor_property("nanite_settings", nanite_settings)
unreal.EditorAssetLibrary.save_loaded_asset(vr_mesh, only_if_is_dirty=False)

world = unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
if not world:
    raise RuntimeError(f"Could not load level: {MAIN_LEVEL_PATH}")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
replaced = []
for actor in actor_subsystem.get_all_level_actors():
    location_before = actor.get_actor_location()
    rotation_before = actor.get_actor_rotation()
    scale_before = actor.get_actor_scale3d()
    changed = False

    for component in actor.get_components_by_class(unreal.StaticMeshComponent):
        if component.get_editor_property("static_mesh") == source_mesh:
            component.modify()
            component.set_editor_property("static_mesh", vr_mesh)
            changed = True

    if changed:
        location_after = actor.get_actor_location()
        rotation_after = actor.get_actor_rotation()
        scale_after = actor.get_actor_scale3d()
        rotation_delta = unreal.MathLibrary.normalized_delta_rotator(rotation_after, rotation_before)
        if (unreal.MathLibrary.vector_distance(location_before, location_after) > 0.01 or
                unreal.MathLibrary.vector_distance(scale_before, scale_after) > 0.0001 or
                abs(rotation_delta.pitch) > 0.01 or abs(rotation_delta.yaw) > 0.01 or
                abs(rotation_delta.roll) > 0.01):
            raise RuntimeError(f"Refusing to save: transformed actor {actor.get_name()}")
        replaced.append(actor.get_name())

if not replaced:
    raise RuntimeError("No L_Main StaticMeshComponent referenced the source Paldalmun mesh")

if not unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True):
    raise RuntimeError("Could not save VR mesh or L_Main")

unreal.log(
    "PALDALMUN_VR_OPTIMIZE SUCCESS: "
    f"mesh={DESTINATION_MESH_PATH} lod_count={lod_count} replaced_actors={replaced}"
)
