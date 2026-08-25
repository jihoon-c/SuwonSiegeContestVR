import unreal


SOURCE_MAP = "/Game/Namhansanseong/Maps/Demo_Namhansanseong"
LANDSCAPE_MAP = "/Game/Maps/Main/L_NamhansanseongLandscape"


def all_actors():
    return unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()


def is_landscape_actor(actor):
    return actor.get_class().get_name() in ("Landscape", "LandscapeStreamingProxy")


if unreal.EditorAssetLibrary.does_asset_exist(LANDSCAPE_MAP):
    if not unreal.EditorAssetLibrary.delete_asset(LANDSCAPE_MAP):
        raise RuntimeError("Could not remove the previous generated landscape sublevel")

unreal.EditorLoadingAndSavingUtils.load_map(SOURCE_MAP)
source_landscapes = [actor for actor in all_actors() if is_landscape_actor(actor)]
if len(source_landscapes) != 122:
    raise RuntimeError(f"Expected 122 source Landscape actors, found {len(source_landscapes)}")

unreal.EditorLoadingAndSavingUtils.load_map("/Engine/Maps/Templates/Template_Default")
if not unreal.EditorAssetLibrary.duplicate_asset(SOURCE_MAP, LANDSCAPE_MAP):
    raise RuntimeError("Could not duplicate the source map for the Landscape sublevel")

unreal.EditorLoadingAndSavingUtils.load_map(LANDSCAPE_MAP)

generated_actors = all_actors()
non_landscape_actors = [actor for actor in generated_actors if not is_landscape_actor(actor)]
unreal.log(
    f"LANDSCAPE_SUBLEVEL source_landscapes={len(source_landscapes)} "
    f"removing_non_landscape={len(non_landscape_actors)}"
)

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not actor_subsystem.destroy_actors(non_landscape_actors):
    raise RuntimeError("Could not remove non-Landscape actors from generated sublevel")

generated_world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if not unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, False):
    raise RuntimeError("Could not save the stripped Landscape sublevel")

unreal.EditorLoadingAndSavingUtils.load_map(LANDSCAPE_MAP)
remaining = all_actors()
remaining_landscapes = [actor for actor in remaining if is_landscape_actor(actor)]
materials = {
    str(actor.get_editor_property("landscape_material"))
    for actor in remaining_landscapes
    if actor.get_editor_property("landscape_material")
}
unreal.log(
    f"LANDSCAPE_SUBLEVEL verify actors={len(remaining)} landscapes={len(remaining_landscapes)} "
    f"materials={sorted(materials)}"
)

if len(remaining) != 122 or len(remaining_landscapes) != 122:
    raise RuntimeError("Generated sublevel did not preserve exactly the 122 Landscape actors")
if not any("MI_Landscape" in material for material in materials):
    raise RuntimeError("Generated sublevel did not preserve MI_Landscape")

unreal.log("LANDSCAPE_SUBLEVEL CREATE SUCCESS")
