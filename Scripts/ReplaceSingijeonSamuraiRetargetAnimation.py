import unreal


ROOT = "/GF_Singijeon/Gameplay/Enemy/Samurai"
OLD_PATH = ROOT + "/MF_Rifle_Jog_Fwd_Samurai"
NEW_PATH = ROOT + "/MF_Rifle_Jog_Fwd_Samurai1"
TARGET_MESH_PATH = ROOT + "/SKM_Low_Poly_Samurai_VR"


old_animation = unreal.load_asset(OLD_PATH)
new_animation = unreal.load_asset(NEW_PATH)
target_mesh = unreal.load_asset(TARGET_MESH_PATH)
if not old_animation or not new_animation or not target_mesh:
    raise RuntimeError("Missing old/new Samurai animation or target mesh")
target_skeleton = target_mesh.get_editor_property("skeleton")
new_skeleton = new_animation.get_editor_property("skeleton")
if not target_skeleton or new_skeleton != target_skeleton:
    raise RuntimeError("New Samurai animation does not use the runtime Skeleton")

asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
if not asset_subsystem.consolidate_assets(new_animation, [old_animation]):
    raise RuntimeError("Could not consolidate the old Samurai animation")
if unreal.EditorAssetLibrary.does_asset_exist(OLD_PATH):
    if not asset_subsystem.delete_asset(OLD_PATH):
        raise RuntimeError("Could not remove the consolidated old Samurai animation")
if not unreal.EditorAssetLibrary.rename_asset(NEW_PATH, OLD_PATH):
    raise RuntimeError("Could not restore canonical Samurai animation name")

canonical_animation = unreal.load_asset(OLD_PATH)
if not canonical_animation:
    raise RuntimeError("Canonical Samurai animation is missing")
unreal.EditorAssetLibrary.save_loaded_asset(canonical_animation, only_if_is_dirty=False)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log(
    "SAMURAI_ANIMATION_REPLACE SUCCESS "
    f"path={canonical_animation.get_path_name()} "
    f"skeleton={canonical_animation.get_editor_property('skeleton').get_path_name()}"
)
