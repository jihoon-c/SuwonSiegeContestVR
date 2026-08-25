import unreal


ROOT = "/GF_Singijeon/Gameplay/Enemy/Samurai"
ORIGINAL_MESH_PATH = "/GF_Singijeon/Asset/EnemyMan1/Low_Poly_Samurai"
OPTIMIZED_MESH_PATH = ROOT + "/SKM_Low_Poly_Samurai_VR"
SOURCE_RIG_PATH = ROOT + "/IK_Manny_Rifle_Source"
TARGET_RIG_PATH = ROOT + "/IK_LowPolySamurai_Target"


def vector_distance(a, b):
    dx = a.x - b.x
    dy = a.y - b.y
    dz = a.z - b.z
    return (dx * dx + dy * dy + dz * dz) ** 0.5


def normalize_mesh_reference_skeleton(mesh_path):
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        raise RuntimeError(f"Missing Samurai mesh: {mesh_path}")
    modifier = unreal.SkeletonModifier()
    if not modifier.set_skeletal_mesh(mesh):
        raise RuntimeError(f"SkeletonModifier rejected: {mesh_path}")

    bone_names = list(modifier.get_all_bone_names())
    roots = [name for name in bone_names if str(modifier.get_parent_name(name)) == "None"]
    if [str(name) for name in roots] != ["Root"]:
        raise RuntimeError(f"Unexpected root hierarchy for {mesh_path}: {roots}")
    root = roots[0]
    root_transform = modifier.get_bone_transform(root, False)
    root_scale = root_transform.scale3d.x
    before_bounds = mesh.get_bounds()
    tracked_bones = ["Root", "Hips", "LeftHand", "RightHand", "LeftFoot", "RightFoot"]
    before_locations = {
        bone: modifier.get_bone_transform(bone, True).translation
        for bone in tracked_bones
    }

    if root_scale > 99.0:
        transforms = []
        for bone_name in bone_names:
            transform = modifier.get_bone_transform(bone_name, False)
            if bone_name == root:
                transform.scale3d = unreal.Vector(1.0, 1.0, 1.0)
            else:
                translation = transform.translation
                transform.translation = unreal.Vector(
                    translation.x * 100.0,
                    translation.y * 100.0,
                    translation.z * 100.0,
                )
            transforms.append(transform)
        if not modifier.set_bones_transforms(bone_names, transforms, False):
            raise RuntimeError(f"Could not normalize reference Skeleton: {mesh_path}")
        for bone, before_location in before_locations.items():
            after_location = modifier.get_bone_transform(bone, True).translation
            if vector_distance(before_location, after_location) > 0.01:
                raise RuntimeError(
                    f"Normalization moved {bone} in {mesh_path}: "
                    f"{before_location} -> {after_location}"
                )
        if not modifier.commit_skeleton_to_skeletal_mesh():
            raise RuntimeError(f"Could not commit normalized Skeleton: {mesh_path}")
        unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
    elif abs(root_scale - 1.0) > 0.01:
        raise RuntimeError(f"Unexpected Root scale {root_scale} in {mesh_path}")

    verifier = unreal.SkeletonModifier()
    if not verifier.set_skeletal_mesh(mesh):
        raise RuntimeError(f"Could not verify normalized mesh: {mesh_path}")
    verified_scale = verifier.get_bone_transform("Root", True).scale3d.x
    after_bounds = mesh.get_bounds()
    if abs(verified_scale - 1.0) > 0.01:
        raise RuntimeError(f"Root scale did not normalize in {mesh_path}: {verified_scale}")
    if vector_distance(before_bounds.origin, after_bounds.origin) > 0.01:
        raise RuntimeError(f"Bounds origin changed in {mesh_path}")
    if vector_distance(before_bounds.box_extent, after_bounds.box_extent) > 0.01:
        raise RuntimeError(f"Bounds extent changed in {mesh_path}")
    unreal.log(
        f"SAMURAI_SKELETON_FIX mesh={mesh_path} root_scale={verified_scale} "
        f"bounds={after_bounds}"
    )
    return mesh


original_mesh = normalize_mesh_reference_skeleton(ORIGINAL_MESH_PATH)
optimized_mesh = normalize_mesh_reference_skeleton(OPTIMIZED_MESH_PATH)
for mesh in (original_mesh, optimized_mesh):
    skeleton = mesh.get_editor_property("skeleton")
    if skeleton:
        unreal.EditorAssetLibrary.save_loaded_asset(skeleton, only_if_is_dirty=False)


def fix_rig(rig_path, root_motion_bone, goals):
    rig = unreal.load_asset(rig_path)
    if not rig:
        raise RuntimeError(f"Missing IK Rig: {rig_path}")
    controller = unreal.IKRigController.get_controller(rig)
    controller.set_root_motion_bone(root_motion_bone)
    for chain_name, goal_name, bone_name in goals:
        if not controller.get_goal(goal_name):
            created_name = controller.add_new_goal(goal_name, bone_name)
            if str(created_name) != goal_name:
                raise RuntimeError(
                    f"Could not add {goal_name} to {rig_path}; got {created_name}"
                )
        if not controller.set_retarget_chain_goal(chain_name, goal_name):
            raise RuntimeError(f"Could not connect {chain_name} to {goal_name}")
    unreal.EditorAssetLibrary.save_loaded_asset(rig, only_if_is_dirty=False)
    for chain_name, goal_name, _ in goals:
        if not controller.get_goal(goal_name):
            raise RuntimeError(f"Goal {goal_name} missing after save in {rig_path}")
        if str(controller.get_retarget_chain_goal(chain_name)) != goal_name:
            raise RuntimeError(f"Chain {chain_name} lost Goal {goal_name}")
    unreal.log(
        f"SAMURAI_IK_FIX rig={rig_path} root_motion={controller.get_root_motion_bone()} "
        f"goals={len(controller.get_all_goals())}"
    )


fix_rig(
    SOURCE_RIG_PATH,
    "root",
    (
        ("LeftLeg", "LeftFootIK", "foot_l"),
        ("RightLeg", "RightFootIK", "foot_r"),
        ("LeftArm", "LeftHandIK", "hand_l"),
        ("RightArm", "RightHandIK", "hand_r"),
    ),
)
fix_rig(
    TARGET_RIG_PATH,
    "Root",
    (
        ("LeftLeg", "LeftFootIK", "LeftFoot"),
        ("RightLeg", "RightFootIK", "RightFoot"),
        ("LeftArm", "LeftHandIK", "LeftHand"),
        ("RightArm", "RightHandIK", "RightHand"),
    ),
)
unreal.log("SAMURAI_SKELETON_IK_FIX SUCCESS")
