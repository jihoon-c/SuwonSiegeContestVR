import unreal


ROOT = "/GF_Singijeon/Gameplay/Enemy/Samurai"
RETARGETER_PATH = ROOT + "/RTG_MannyRifle_To_LowPolySamurai"
SOURCE_RIG_PATH = ROOT + "/IK_Manny_Rifle_Source"
TARGET_RIG_PATH = ROOT + "/IK_LowPolySamurai_Target"
SOURCE_MESH_PATH = "/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Meshes/SKM_Quinn_Simple"
ORIGINAL_TARGET_MESH_PATH = "/GF_Singijeon/Asset/EnemyMan1/Low_Poly_Samurai"
OPTIMIZED_TARGET_MESH_PATH = ROOT + "/SKM_Low_Poly_Samurai_VR"

retargeter = unreal.load_asset(RETARGETER_PATH)
source_rig = unreal.load_asset(SOURCE_RIG_PATH)
target_rig = unreal.load_asset(TARGET_RIG_PATH)
source_mesh = unreal.load_asset(SOURCE_MESH_PATH)
original_target_mesh = unreal.load_asset(ORIGINAL_TARGET_MESH_PATH)
optimized_target_mesh = unreal.load_asset(OPTIMIZED_TARGET_MESH_PATH)

for label, asset in (
    ("retargeter", retargeter),
    ("source_rig", source_rig),
    ("target_rig", target_rig),
    ("source_mesh", source_mesh),
    ("original_target_mesh", original_target_mesh),
    ("optimized_target_mesh", optimized_target_mesh),
):
    unreal.log(
        f"SAMURAI_RTG_INSPECT {label}="
        f"{asset.get_path_name() if asset else 'None'}"
    )

if not retargeter or not source_rig or not target_rig:
    raise RuntimeError("Required Samurai retarget authoring assets are missing")

controller = unreal.IKRetargeterController.get_controller(retargeter)
for prop in (
    "source_mesh_offset",
    "target_mesh_offset",
    "target_mesh_scale",
    "show_source_mesh",
    "show_target_mesh",
    "show_source_skeleton",
    "show_target_skeleton",
):
    try:
        unreal.log(
            f"SAMURAI_RTG_INSPECT RETARGETER_{prop}="
            f"{retargeter.get_editor_property(prop)}"
        )
    except Exception as exc:
        unreal.log_warning(
            f"SAMURAI_RTG_INSPECT RETARGETER_{prop}_FAILED={exc}"
        )
unreal.log(
    "SAMURAI_RTG_INSPECT CONTROLLER_PREVIEW_API="
    + ",".join(
        name for name in dir(controller)
        if "preview" in name.lower() or "offset" in name.lower() or "scale" in name.lower()
    )
)
mesh_editor = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
unreal.log(
    "SAMURAI_RTG_INSPECT MESH_EDITOR_API="
    + ",".join(
        name for name in dir(mesh_editor)
        if "lod" in name.lower() or "vert" in name.lower() or "section" in name.lower()
    )
)
for side in (unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET):
    side_name = "SOURCE" if side == unreal.RetargetSourceOrTarget.SOURCE else "TARGET"
    assigned_rig = controller.get_ik_rig(side)
    preview_mesh = controller.get_preview_mesh(side)
    unreal.log(
        f"SAMURAI_RTG_INSPECT {side_name} "
        f"rig={assigned_rig.get_path_name() if assigned_rig else 'None'} "
        f"preview={preview_mesh.get_path_name() if preview_mesh else 'None'}"
    )
    try:
        unreal.log(
            f"SAMURAI_RTG_INSPECT {side_name}_ROOT_POSE_OFFSET="
            f"{controller.get_root_offset_in_retarget_pose(side)}"
        )
    except Exception as exc:
        unreal.log_warning(
            f"SAMURAI_RTG_INSPECT {side_name}_ROOT_POSE_OFFSET_FAILED={exc}"
        )

for label, rig in (("SOURCE", source_rig), ("TARGET", target_rig)):
    rig_controller = unreal.IKRigController.get_controller(rig)
    mesh = rig_controller.get_skeletal_mesh()
    chains = list(rig_controller.get_retarget_chains())
    root = rig_controller.get_retarget_root()
    unreal.log(
        f"SAMURAI_RTG_INSPECT {label}_IK_RIG_MESH="
        f"{mesh.get_path_name() if mesh else 'None'} "
        f"root={root} chains={len(chains)}"
    )
    goals = list(rig_controller.get_all_goals())
    unreal.log(
        f"SAMURAI_RTG_INSPECT {label}_GOALS="
        + ",".join(
            f"{goal.get_editor_property('goal_name')}->"
            f"{goal.get_editor_property('bone_name')}"
            for goal in goals
        )
    )
    root_motion_bone = rig_controller.get_root_motion_bone()
    bones_to_inspect = {str(root), str(root_motion_bone)}
    unreal.log(
        f"SAMURAI_RTG_INSPECT {label}_ROOT_MOTION_BONE={root_motion_bone}"
    )
    for chain in chains:
        chain_name = str(chain.chain_name)
        start_bone = str(rig_controller.get_retarget_chain_start_bone(chain_name))
        end_bone = str(rig_controller.get_retarget_chain_end_bone(chain_name))
        goal_name = str(rig_controller.get_retarget_chain_goal(chain_name))
        bones_to_inspect.add(start_bone)
        bones_to_inspect.add(end_bone)
        unreal.log(
            f"SAMURAI_RTG_INSPECT {label}_CHAIN name={chain_name} "
            f"start={start_bone} end={end_bone} goal={goal_name} "
            f"goal_exists={rig_controller.get_goal(goal_name) is not None if goal_name != 'None' else True}"
        )
    for bone_name in sorted(bones_to_inspect):
        if not bone_name or bone_name == "None":
            continue
        try:
            unreal.log(
                f"SAMURAI_RTG_INSPECT {label}_REF_POSE bone={bone_name} "
                f"transform={rig_controller.get_ref_pose_transform_of_bone(bone_name)}"
            )
        except Exception as exc:
            unreal.log_warning(
                f"SAMURAI_RTG_INSPECT {label}_REF_POSE_FAILED bone={bone_name} error={exc}"
            )

for label, mesh in (
    ("source", source_mesh),
    ("original_target", original_target_mesh),
    ("optimized_target", optimized_target_mesh),
):
    skeleton = mesh.get_editor_property("skeleton") if mesh else None
    unreal.log(
        f"SAMURAI_RTG_INSPECT {label}_skeleton="
        f"{skeleton.get_path_name() if skeleton else 'None'}"
    )
    if mesh:
        bounds = mesh.get_bounds()
        unreal.log(
            f"SAMURAI_RTG_INSPECT {label}_bounds "
            f"origin={bounds.origin} extent={bounds.box_extent} radius={bounds.sphere_radius}"
        )
        try:
            lod_count = mesh_editor.get_lod_count(mesh)
            unreal.log(f"SAMURAI_RTG_INSPECT {label}_lod_count={lod_count}")
            for lod_index in range(lod_count):
                unreal.log(
                    f"SAMURAI_RTG_INSPECT {label}_lod[{lod_index}] "
                    f"verts={mesh_editor.get_num_verts(mesh, lod_index)} "
                    f"sections={mesh_editor.get_num_sections(mesh, lod_index)}"
                )
        except Exception as exc:
            unreal.log_warning(
                f"SAMURAI_RTG_INSPECT {label}_lod_query_failed={exc}"
            )
        for prop in ("min_lod", "lod_info", "materials", "ray_tracing_min_lod"):
            try:
                unreal.log(
                    f"SAMURAI_RTG_INSPECT {label}_{prop}="
                    f"{mesh.get_editor_property(prop)}"
                )
            except Exception:
                pass
        try:
            import_data = mesh.get_editor_property("asset_import_data")
            unreal.log(
                f"SAMURAI_RTG_INSPECT {label}_IMPORT_DATA_CLASS="
                f"{import_data.get_class().get_name()}"
            )
            unreal.log(
                f"SAMURAI_RTG_INSPECT {label}_IMPORT_SOURCE="
                f"{import_data.get_first_filename()}"
            )
            unreal.log(
                f"SAMURAI_RTG_INSPECT {label}_IMPORT_API="
                + ",".join(
                    name for name in dir(import_data)
                    if "scale" in name.lower() or "transform" in name.lower()
                    or "unit" in name.lower() or "scene" in name.lower()
                )
            )
        except Exception as exc:
            unreal.log_warning(
                f"SAMURAI_RTG_INSPECT {label}_IMPORT_QUERY_FAILED={exc}"
            )

unreal.log("SAMURAI_RTG_INSPECT SUCCESS")
