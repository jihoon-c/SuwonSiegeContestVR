import unreal

SOURCE = "/GF_Singijeon/Asset/EnemyMan1/Low_Poly_Samurai"
TARGET = "/GF_Singijeon/Gameplay/Enemy/Samurai/SKM_Low_Poly_Samurai_VR"
source = unreal.load_asset(SOURCE)
if not source:
    raise RuntimeError("Low Poly Samurai source mesh is missing")
mesh = unreal.load_asset(TARGET)
if not mesh:
    mesh = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset(
        "SKM_Low_Poly_Samurai_VR", "/GF_Singijeon/Gameplay/Enemy/Samurai", source)
if not mesh:
    raise RuntimeError("Could not duplicate Samurai VR mesh")
subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
if not subsystem.regenerate_lod(mesh, 3, False, False):
    raise RuntimeError("Could not generate Samurai skeletal LODs")
unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
unreal.log(f"SAMURAI_LOD SUCCESS count={subsystem.get_lod_count(mesh)}")
