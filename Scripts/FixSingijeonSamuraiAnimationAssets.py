import unreal


MESH_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/SKM_Low_Poly_Samurai_VR"
MATERIAL_PATH = "/GF_Singijeon/Asset/EnemyMan1/material_0"


mesh = unreal.load_asset(MESH_PATH)
material = unreal.load_asset(MATERIAL_PATH)
if not mesh:
    raise RuntimeError(f"Missing Samurai mesh: {MESH_PATH}")
if not material:
    raise RuntimeError(f"Missing Samurai material: {MATERIAL_PATH}")

mesh_editor = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
lod_count = mesh_editor.get_lod_count(mesh)
if lod_count <= 0:
    raise RuntimeError("Samurai mesh has no LODs")

for lod_index in range(lod_count):
    build_settings = mesh_editor.get_lod_build_settings(mesh, lod_index)
    if not build_settings.get_editor_property("optimize_for_instancing"):
        build_settings.set_editor_property("optimize_for_instancing", True)
        mesh_editor.set_lod_build_settings(mesh, lod_index, build_settings)

unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)

# EMaterialUsage::MATUSAGE_InstancedSkinnedMesh is index 26 in UE 5.8.
# Override it on the project-owned material instance instead of changing the
# shared Interchange parent material for the entire engine installation.
instanced_skinned_mesh_usage = 1 << 26
overrides = material.get_editor_property("base_property_overrides")
overrides.set_editor_property(
    "override_usage_flags",
    int(overrides.get_editor_property("override_usage_flags")) |
    instanced_skinned_mesh_usage,
)
overrides.set_editor_property(
    "usage_flags",
    int(overrides.get_editor_property("usage_flags")) |
    instanced_skinned_mesh_usage,
)
material.set_editor_property("base_property_overrides", overrides)
material.modify()
unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log(
    f"SAMURAI_ANIMATION_FIX SUCCESS lods={lod_count} "
    "optimize_for_instancing=true material_usage=true"
)
