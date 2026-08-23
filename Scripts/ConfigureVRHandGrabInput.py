import unreal


HAND_CONTEXT_PATH = "/Game/XRFramework/Input/IMC_Hands"
LEFT_ACTION_PATH = "/Game/XRFramework/Input/Actions/Hands/IA_Hand_IndexCurl_Left"
RIGHT_ACTION_PATH = "/Game/XRFramework/Input/Actions/Hands/IA_Hand_IndexCurl_Right"


hand_context = unreal.load_asset(HAND_CONTEXT_PATH)
left_action = unreal.load_asset(LEFT_ACTION_PATH)
right_action = unreal.load_asset(RIGHT_ACTION_PATH)
if not hand_context or not left_action or not right_action:
    raise RuntimeError("IMC_Hands or hand trigger Input Actions are missing")


def make_key(key_name):
    key = unreal.Key()
    key.set_editor_property("key_name", unreal.Name(key_name))
    return key


def make_mapping(action, key_name):
    mapping = unreal.EnhancedActionKeyMapping()
    mapping.set_editor_property("action", action)
    mapping.set_editor_property("key", make_key(key_name))
    return mapping


managed_actions = {left_action.get_path_name(), right_action.get_path_name()}
mappings = [
    mapping
    for mapping in hand_context.get_editor_property("mappings")
    if mapping.get_editor_property("action").get_path_name() not in managed_actions
]
mappings.extend(
    [
        make_mapping(left_action, "OculusTouch_Left_Trigger_Axis"),
        make_mapping(right_action, "OculusTouch_Right_Trigger_Axis"),
    ]
)
hand_context.set_editor_property("mappings", mappings)
unreal.EditorAssetLibrary.save_loaded_asset(hand_context, only_if_is_dirty=False)
unreal.log("VR_HAND_GRAB_INPUT CONFIGURE SUCCESS")
