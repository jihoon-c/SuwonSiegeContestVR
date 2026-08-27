"""Create the floating, flat 4x4-component Landscape in L_Main."""

import unreal


MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"
FLAT_TAG = "FlatLandscape4x4"
LANDSCAPE_MATERIAL_PATH = "/Game/Namhansanseong/Materials/Landscape/MI_Landscape"

world = unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
if not world:
    raise RuntimeError(f"Could not load {MAIN_LEVEL_PATH}")

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
existing = [actor for actor in actors
            if isinstance(actor, unreal.Landscape) and FLAT_TAG in [str(tag) for tag in actor.tags]]
if not existing:
    unreal.SystemLibrary.execute_console_command(world, "Suwon.CreateFlatLandscape4x4")

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
flat_landscapes = [actor for actor in actors
                   if isinstance(actor, unreal.Landscape) and FLAT_TAG in [str(tag) for tag in actor.tags]]
if len(flat_landscapes) != 1:
    raise RuntimeError(f"Expected one FlatLandscape4x4 actor, found {len(flat_landscapes)}")

flat = flat_landscapes[0]
location = flat.get_actor_location()

if flat.get_editor_property("landscape_material") is None:
    material_asset = unreal.load_asset(LANDSCAPE_MATERIAL_PATH)
    if material_asset is None:
        raise RuntimeError(f"Could not load landscape material: {LANDSCAPE_MATERIAL_PATH}")
    flat.set_editor_property("landscape_material", material_asset)
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
material = flat.get_editor_property("landscape_material")
if material is None or "MI_Landscape" not in material.get_path_name():
    raise RuntimeError(f"Flat landscape material was not applied: {material}")

unreal.log(
    f"FLAT_LANDSCAPE_4X4 VERIFY SUCCESS actor={flat.get_name()} "
    f"location={location} scale={flat.get_actor_scale3d()} material={material.get_path_name()}"
)
