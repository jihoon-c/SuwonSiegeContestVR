import unreal

unreal.log(f"VR_GRAB_INSPECT Key doc={unreal.Key.__doc__}")
unreal.log(f"VR_GRAB_INSPECT Mapping doc={unreal.EnhancedActionKeyMapping.__doc__}")
unreal.log(f"VR_GRAB_INSPECT Key fields={dir(unreal.Key())}")
unreal.log(f"VR_GRAB_INSPECT Mapping fields={dir(unreal.EnhancedActionKeyMapping())}")


def log_mapping_context(path):
    context = unreal.load_asset(path)
    unreal.log(f"VR_GRAB_INSPECT context={path} asset={context}")
    if not context:
        return
    for mapping in context.get_editor_property("mappings"):
        action = mapping.get_editor_property("action")
        key = mapping.get_editor_property("key")
        unreal.log(
            f"VR_GRAB_INSPECT mapping action={action.get_path_name() if action else None} key={key}"
        )


for context_path in (
    "/Game/XRFramework/Input/IMC_Hands",
    "/Game/XRFramework/Input/IMC_Default",
):
    log_mapping_context(context_path)

pawn_bp = unreal.load_asset("/Game/Core/VR/Pawn/BP_VRPlayerPawn")
if pawn_bp:
    pawn_cdo = unreal.get_default_object(pawn_bp.generated_class())
    for prop in (
        "trigger_grab_left_action",
        "trigger_grab_right_action",
        "grab_left_action",
        "grab_right_action",
        "default_mapping_context",
    ):
        try:
            unreal.log(f"VR_GRAB_INSPECT pawn.{prop}={pawn_cdo.get_editor_property(prop)}")
        except Exception as error:
            unreal.log_warning(f"VR_GRAB_INSPECT pawn.{prop} unavailable: {error}")
    for component_name in ("MotionControllerLeftGrip", "MotionControllerRightGrip"):
        components = [
            item for item in pawn_cdo.get_components_by_class(unreal.MotionControllerComponent)
            if item.get_name() == component_name
        ]
        for component in components:
            unreal.log(
                f"VR_GRAB_INSPECT {component_name}.motion_source="
                f"{component.get_editor_property('motion_source')}"
            )

arrow_mesh = unreal.load_asset(
    "/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb"
)
arrow_material = unreal.load_asset(
    "/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_Arrow01b"
)
unreal.log(f"SINGIJEON_MATERIAL_INSPECT mesh={arrow_mesh} material={arrow_material}")
if arrow_mesh:
    for index, static_material in enumerate(arrow_mesh.get_editor_property("static_materials")):
        unreal.log(
            f"SINGIJEON_MATERIAL_INSPECT mesh_slot={index} "
            f"material={static_material.get_editor_property('material_interface')}"
        )
if arrow_material:
    try:
        unreal.log(
            f"SINGIJEON_MATERIAL_INSPECT parent={arrow_material.get_editor_property('parent')}"
        )
    except Exception as error:
        unreal.log_warning(f"SINGIJEON_MATERIAL_INSPECT parent unavailable: {error}")

hwacha_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
if hwacha_bp:
    hwacha_cdo = unreal.get_default_object(hwacha_bp.generated_class())
    unreal.log(
        f"SINGIJEON_MATERIAL_INSPECT auto_fill_material="
        f"{hwacha_cdo.get_editor_property('auto_fill_arrow_material')}"
    )
    instances = hwacha_cdo.get_editor_property("auto_loaded_arrow_instances")
    unreal.log(
        f"SINGIJEON_MATERIAL_INSPECT ism_mesh={instances.get_editor_property('static_mesh')}"
    )
    for index in range(max(1, len(arrow_mesh.get_editor_property("static_materials")))):
        unreal.log(
            f"SINGIJEON_MATERIAL_INSPECT ism_slot={index} material={instances.get_material(index)}"
        )

unreal.log("VR_GRAB_AND_SINGIJEON_MATERIAL_INSPECTION COMPLETE")
