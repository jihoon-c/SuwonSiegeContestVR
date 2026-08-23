import unreal

ROOT = "/GF_Singijeon/Gameplay/Enemy/Samurai"
SOURCE_MESH_PATH = "/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Meshes/SKM_Quinn_Simple"
TARGET_MESH_PATH = "/GF_Singijeon/Asset/EnemyMan1/Low_Poly_Samurai"
SOURCE_ANIM_PATH = "/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd"

unreal.EditorAssetLibrary.make_directory(ROOT)
tools = unreal.AssetToolsHelpers.get_asset_tools()
source_mesh = unreal.load_asset(SOURCE_MESH_PATH)
target_mesh = unreal.load_asset(TARGET_MESH_PATH)
source_anim = unreal.load_asset(SOURCE_ANIM_PATH)
if not source_mesh or not target_mesh or not source_anim:
    raise RuntimeError("Required source/target assets are missing")


def create_ik_rig(name, mesh):
    path = f"{ROOT}/{name}"
    rig = unreal.load_asset(path)
    if not rig:
        rig = tools.create_asset(name, ROOT, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    if not rig:
        raise RuntimeError(f"Could not create {name}")
    controller = unreal.IKRigController.get_controller(rig)
    controller.set_skeletal_mesh(mesh)
    if not controller.apply_auto_generated_retarget_definition():
        raise RuntimeError(f"Auto characterizer could not define {name}")
    unreal.EditorAssetLibrary.save_loaded_asset(rig, only_if_is_dirty=False)
    return rig


source_rig = create_ik_rig("IK_Manny_Rifle_Source", source_mesh)
target_rig = create_ik_rig("IK_LowPolySamurai_Target", target_mesh)

retargeter_path = f"{ROOT}/RTG_MannyRifle_To_LowPolySamurai"
retargeter = unreal.load_asset(retargeter_path)
if not retargeter:
    retargeter = tools.create_asset(
        "RTG_MannyRifle_To_LowPolySamurai", ROOT,
        unreal.IKRetargeter, unreal.IKRetargetFactory())
if not retargeter:
    raise RuntimeError("Could not create IK Retargeter")
controller = unreal.IKRetargeterController.get_controller(retargeter)
controller.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, source_rig)
controller.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, target_rig)
controller.set_preview_mesh(unreal.RetargetSourceOrTarget.SOURCE, source_mesh)
controller.set_preview_mesh(unreal.RetargetSourceOrTarget.TARGET, target_mesh)
controller.remove_all_ops()
controller.add_default_ops()
controller.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
unreal.EditorAssetLibrary.save_loaded_asset(retargeter, only_if_is_dirty=False)

results = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
    [unreal.EditorAssetLibrary.find_asset_data(SOURCE_ANIM_PATH)], source_mesh, target_mesh, retargeter,
    suffix="_Samurai", target_path=ROOT,
    use_source_path=False, include_referenced_assets=False,
    overwrite_existing_files=True)
if not results:
    raise RuntimeError("Rifle Jog retarget produced no animation")
for result in results:
    unreal.log(f"SAMURAI_RETARGET RESULT {result.package_name}")
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("SAMURAI_RETARGET SUCCESS")
