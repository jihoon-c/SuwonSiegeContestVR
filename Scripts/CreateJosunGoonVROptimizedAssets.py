import unreal


SOURCE_POINT_MESH = "/GF_Singijeon/Asset/JosunGoonPoint/Pointing"
SOURCE_POINT_ANIM = "/GF_Singijeon/Asset/JosunGoonPoint/Pointing_Anim"
SOURCE_KNEEL_MESH = "/GF_Singijeon/Asset/JosunGoonKneel/Kneeling"
SOURCE_KNEEL_ANIM = "/GF_Singijeon/Asset/JosunGoonKneel/Kneeling_Anim"
TARGET_DIR = "/GF_Singijeon/Gameplay/Characters/JosunGoon"
TARGET_MESH = f"{TARGET_DIR}/SKM_JosunGoon_VR"
TARGET_POINT_ANIM = f"{TARGET_DIR}/A_JosunGoon_Point_VR"
TARGET_KNEEL_ANIM = f"{TARGET_DIR}/A_JosunGoon_Kneel_VR"


def require(path, expected_type):
    asset = unreal.load_asset(path)
    if not isinstance(asset, expected_type):
        raise RuntimeError(f"Missing or invalid asset: {path}")
    return asset


def duplicate_or_load(source_path, target_path):
    existing = unreal.load_asset(target_path)
    if existing:
        return existing
    if not unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path):
        raise RuntimeError(f"Could not duplicate {source_path} to {target_path}")
    result = unreal.load_asset(target_path)
    if not result:
        raise RuntimeError(f"Duplicated asset could not be loaded: {target_path}")
    return result


point_mesh = require(SOURCE_POINT_MESH, unreal.SkeletalMesh)
kneel_mesh = require(SOURCE_KNEEL_MESH, unreal.SkeletalMesh)
point_anim = require(SOURCE_POINT_ANIM, unreal.AnimSequence)
kneel_anim = require(SOURCE_KNEEL_ANIM, unreal.AnimSequence)
point_skeleton = point_mesh.get_editor_property("skeleton")
kneel_skeleton = kneel_mesh.get_editor_property("skeleton")
if not point_skeleton or not kneel_skeleton:
    raise RuntimeError("JosunGoon source mesh has no Skeleton")

# Both imports contain the same character mesh but were imported into separate
# Skeleton assets. Only unify them when track order and reference pose match.
point_tracks = list(unreal.AnimationLibrary.get_animation_track_names(point_anim))
kneel_tracks = list(unreal.AnimationLibrary.get_animation_track_names(kneel_anim))
if point_tracks != kneel_tracks:
    raise RuntimeError("Kneel/Point animation bone tracks differ; IK retargeting is required")

point_pose = point_skeleton.get_reference_pose()
kneel_pose = kneel_skeleton.get_reference_pose()
point_bones = list(point_pose.get_bone_names())
kneel_bones = list(kneel_pose.get_bone_names())
if point_bones != kneel_bones:
    raise RuntimeError("Kneel/Point reference pose bone hierarchy differs")
different_pose_bones = []
for index, bone_name in enumerate(point_bones):
    point_transform = point_pose.get_bone_pose(bone_name)
    kneel_transform = kneel_pose.get_bone_pose(bone_name)
    if not point_transform.equals(kneel_transform):
        different_pose_bones.append((index, bone_name))

unreal.EditorAssetLibrary.make_directory(TARGET_DIR)
vr_mesh = duplicate_or_load(SOURCE_POINT_MESH, TARGET_MESH)
if not isinstance(vr_mesh, unreal.SkeletalMesh):
    raise RuntimeError("SKM_JosunGoon_VR is not a SkeletalMesh")

mesh_editor = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
if not mesh_editor.regenerate_lod(vr_mesh, 3, False, False):
    raise RuntimeError("Could not generate three JosunGoon VR LODs")
for lod_index in range(mesh_editor.get_lod_count(vr_mesh)):
    build_settings = mesh_editor.get_lod_build_settings(vr_mesh, lod_index)
    build_settings.set_editor_property("optimize_for_instancing", True)
    mesh_editor.set_lod_build_settings(vr_mesh, lod_index, build_settings)

point_vr_anim = duplicate_or_load(SOURCE_POINT_ANIM, TARGET_POINT_ANIM)
if not isinstance(point_vr_anim, unreal.AnimSequence):
    raise RuntimeError("Point VR animation duplicate is invalid")

# Bone names match but reference transforms differ, so use an actual IK
# Retargeter instead of directly replacing Skeleton references.
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def create_ik_rig(name, mesh, root_bone):
    path = f"{TARGET_DIR}/{name}"
    rig = unreal.load_asset(path)
    if not rig:
        rig = asset_tools.create_asset(
            name, TARGET_DIR, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    if not rig:
        raise RuntimeError(f"Could not create {name}")
    controller = unreal.IKRigController.get_controller(rig)
    controller.set_skeletal_mesh(mesh)
    if not controller.apply_auto_generated_retarget_definition():
        raise RuntimeError(f"Auto characterizer could not define {name}")
    controller.set_root_motion_bone(root_bone)
    unreal.EditorAssetLibrary.save_loaded_asset(rig, only_if_is_dirty=False)
    return rig


root_bone = str(point_bones[0])
source_rig = create_ik_rig("IK_JosunGoonKneel_Source", kneel_mesh, root_bone)
target_rig = create_ik_rig("IK_JosunGoonPoint_Target", vr_mesh, root_bone)
retargeter_path = f"{TARGET_DIR}/RTG_JosunGoonKneel_To_Point"
retargeter = unreal.load_asset(retargeter_path)
if not retargeter:
    retargeter = asset_tools.create_asset(
        "RTG_JosunGoonKneel_To_Point", TARGET_DIR,
        unreal.IKRetargeter, unreal.IKRetargetFactory())
if not retargeter:
    raise RuntimeError("Could not create JosunGoon IK Retargeter")
retarget_controller = unreal.IKRetargeterController.get_controller(retargeter)
retarget_controller.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, source_rig)
retarget_controller.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, target_rig)
retarget_controller.set_preview_mesh(unreal.RetargetSourceOrTarget.SOURCE, kneel_mesh)
retarget_controller.set_preview_mesh(unreal.RetargetSourceOrTarget.TARGET, vr_mesh)
retarget_controller.remove_all_ops()
retarget_controller.add_default_ops()
retarget_controller.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
unreal.EditorAssetLibrary.save_loaded_asset(retargeter, only_if_is_dirty=False)

retarget_results = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
    [unreal.EditorAssetLibrary.find_asset_data(SOURCE_KNEEL_ANIM)],
    kneel_mesh,
    vr_mesh,
    retargeter,
    search="Kneeling_Anim",
    replace="A_JosunGoon_Kneel_VR",
    target_path=TARGET_DIR,
    use_source_path=False,
    include_referenced_assets=False,
    overwrite_existing_files=True,
)
if not retarget_results:
    raise RuntimeError("Kneel IK retarget produced no animation")
kneel_vr_anim = unreal.load_asset(TARGET_KNEEL_ANIM)
if not isinstance(kneel_vr_anim, unreal.AnimSequence):
    raise RuntimeError("Retargeted Kneel VR animation is invalid")

for asset in (vr_mesh, point_vr_anim, kneel_vr_anim, source_rig, target_rig, retargeter):
    unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

unreal.log(
    "JOSUN_GOON_VR_CREATE SUCCESS "
    f"mesh={TARGET_MESH} lods={mesh_editor.get_lod_count(vr_mesh)} "
    f"bones={len(point_bones)} tracks={len(point_tracks)} "
    f"converted_ref_pose_bones={len(different_pose_bones)} "
    f"point_length={point_vr_anim.get_play_length():.3f} "
    f"kneel_length={kneel_vr_anim.get_play_length():.3f}"
)
