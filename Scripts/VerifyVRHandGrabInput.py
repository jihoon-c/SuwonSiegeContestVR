import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"VR_HAND_GRAB_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"VR_HAND_GRAB_VERIFY FAIL: {message}")


context = unreal.load_asset("/Game/XRFramework/Input/IMC_Hands")
left_action = unreal.load_asset(
    "/Game/XRFramework/Input/Actions/Hands/IA_Hand_IndexCurl_Left"
)
right_action = unreal.load_asset(
    "/Game/XRFramework/Input/Actions/Hands/IA_Hand_IndexCurl_Right"
)
check(context is not None, "IMC_Hands loads")
check(left_action is not None, "Left trigger Input Action loads")
check(right_action is not None, "Right trigger Input Action loads")

player_pawn_bp = unreal.load_asset("/Game/Core/VR/Pawn/BP_VRPlayerPawn")
check(player_pawn_bp is not None, "BP_VRPlayerPawn loads")
if player_pawn_bp and context:
    pawn_cdo = unreal.get_default_object(player_pawn_bp.generated_class())
    check(
        pawn_cdo.get_editor_property("hand_mapping_context") == context,
        "BP_VRPlayerPawn inherits IMC_Hands as its hand mapping context",
    )

if context:
    resolved = {
        (
            mapping.get_editor_property("action").get_path_name(),
            str(mapping.get_editor_property("key").get_editor_property("key_name")),
        )
        for mapping in context.get_editor_property("mappings")
        if mapping.get_editor_property("action")
    }
    check(
        (left_action.get_path_name(), "OculusTouch_Left_Trigger_Axis") in resolved,
        "Left Quest trigger is mapped to the left grab action",
    )
    check(
        (right_action.get_path_name(), "OculusTouch_Right_Trigger_Axis") in resolved,
        "Right Quest trigger is mapped to the right grab action",
    )

if errors:
    raise RuntimeError("VR hand grab verification failed: " + "; ".join(errors))
unreal.log("VR_HAND_GRAB_INPUT VERIFY SUCCESS")
