import unreal


ROOT = "/GF_Singijeon/Gameplay/Enemy/Samurai"
RETARGETER_PATH = ROOT + "/RTG_MannyRifle_To_LowPolySamurai"
SOURCE_RIG_PATH = ROOT + "/IK_Manny_Rifle_Source"
TARGET_RIG_PATH = ROOT + "/IK_LowPolySamurai_Target"
SOURCE_MESH_PATH = "/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Meshes/SKM_Quinn_Simple"
TARGET_MESH_PATH = "/GF_Singijeon/Asset/EnemyMan1/Low_Poly_Samurai"


def require(asset, path):
    if not asset:
        raise RuntimeError(f"Missing required asset: {path}")
    return asset


retargeter = require(unreal.load_asset(RETARGETER_PATH), RETARGETER_PATH)
source_rig = require(unreal.load_asset(SOURCE_RIG_PATH), SOURCE_RIG_PATH)
target_rig = require(unreal.load_asset(TARGET_RIG_PATH), TARGET_RIG_PATH)
source_mesh = require(unreal.load_asset(SOURCE_MESH_PATH), SOURCE_MESH_PATH)
target_mesh = require(unreal.load_asset(TARGET_MESH_PATH), TARGET_MESH_PATH)

target_skeleton = target_mesh.get_editor_property("skeleton")
rig_controller = unreal.IKRigController.get_controller(target_rig)
current_rig_mesh = rig_controller.get_skeletal_mesh()
if current_rig_mesh:
    current_skeleton = current_rig_mesh.get_editor_property("skeleton")
    if current_skeleton != target_skeleton:
        raise RuntimeError("Target IK Rig and original Samurai use different Skeletons")

# Keep the imported source mesh in the IK Retargeter viewport. The runtime LOD
# copy shares this Skeleton, but its regenerated render data is not reliable in
# the IK Retargeter preview viewport in UE 5.8.
rig_controller.set_skeletal_mesh(target_mesh)

controller = unreal.IKRetargeterController.get_controller(retargeter)
controller.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, source_rig)
controller.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, target_rig)
controller.set_preview_mesh(unreal.RetargetSourceOrTarget.SOURCE, source_mesh)
controller.set_preview_mesh(unreal.RetargetSourceOrTarget.TARGET, target_mesh)
controller.auto_map_chains(unreal.AutoMapChainType.EXACT, True)

# A zero TargetMeshScale collapses the preview mesh to a single point. Keep
# these editor-only values deterministic because the Retargeter editor can
# retain stale values in an already-open tab.
retargeter.set_editor_property("source_mesh_offset", unreal.Vector(0.0, 0.0, 0.0))
retargeter.set_editor_property("target_mesh_offset", unreal.Vector(160.0, 0.0, 0.0))
retargeter.set_editor_property("target_mesh_scale", 1.816)
retargeter.set_editor_property("show_source_mesh", True)
retargeter.set_editor_property("show_target_mesh", True)
retargeter.set_editor_property("show_source_skeleton", True)
retargeter.set_editor_property("show_target_skeleton", True)

if controller.get_ik_rig(unreal.RetargetSourceOrTarget.TARGET) != target_rig:
    raise RuntimeError("Target IK Rig reference was not accepted by the Retargeter")
if controller.get_preview_mesh(unreal.RetargetSourceOrTarget.TARGET) != target_mesh:
    raise RuntimeError("Original Samurai preview mesh was not accepted by the Retargeter")
if rig_controller.get_skeletal_mesh() != target_mesh:
    raise RuntimeError("Original Samurai mesh was not accepted by the Target IK Rig")
if retargeter.get_editor_property("target_mesh_scale") < 0.01:
    raise RuntimeError("Target preview mesh scale remained zero")

unreal.EditorAssetLibrary.save_loaded_asset(target_rig, only_if_is_dirty=False)
unreal.EditorAssetLibrary.save_loaded_asset(retargeter, only_if_is_dirty=False)
unreal.log(
    "SAMURAI_RTG_FIX SUCCESS: target_ik_rig=IK_LowPolySamurai_Target "
    "target_preview=Low_Poly_Samurai target_offset=(160,0,0) target_scale=1.816"
)
