import unreal


HWACHA_BP_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonHwacha"
HANDLE_MESH_PATH = "/Engine/BasicShapes/Cylinder"
HANDLE_MATERIAL_PATH = (
    "/Game/LevelPrototyping/Interactable/JumpPad/Assets/Materials/MI_GlowNT"
)

hwacha_bp = unreal.load_asset(HWACHA_BP_PATH)
handle_mesh = unreal.load_asset(HANDLE_MESH_PATH)
handle_material = unreal.load_asset(HANDLE_MATERIAL_PATH)
if not hwacha_bp or not handle_mesh or not handle_material:
    raise RuntimeError("Hwacha Blueprint, handle mesh, or highlight material is missing")

hwacha_cdo = unreal.get_default_object(hwacha_bp.generated_class())
hwacha_cdo.set_editor_property("enable_aim_guide_highlight", True)
hwacha_cdo.set_editor_property("show_move_target_marker", True)
hwacha_cdo.set_editor_property("move_target_acceptance_radius", 55.0)

# The imported Hwacha mesh runs from about X=-85..183, Y=-69..69 and Z=0..178.
# The slim handle proxies remain the grab guide and hide while either hand carries.
# The move target below separately communicates the destination.
handle_settings = (
    ("left_handle_highlight", unreal.Vector(-47.0, -43.0, 61.0)),
    ("right_handle_highlight", unreal.Vector(-47.0, 43.0, 61.0)),
)
for property_name, location in handle_settings:
    component = hwacha_cdo.get_editor_property(property_name)
    component.set_static_mesh(handle_mesh)
    component.set_material(0, handle_material)
    component.set_editor_property("relative_location", location)
    component.set_editor_property("relative_rotation", unreal.Rotator(90.0, 0.0, 0.0))
    component.set_editor_property("relative_scale3d", unreal.Vector(0.075, 0.075, 0.72))
    component.set_editor_property("visible", False)
    component.set_editor_property("hidden_in_game", True)
    component.set_editor_property("component_tags", [unreal.Name("VRGrab")])

# This marker is authored relative to the Hwacha for easy placement, then the
# C++ actor detaches it at BeginPlay so it stays fixed while the cart is dragged.
target_marker = hwacha_cdo.get_editor_property("move_target_marker")
target_marker.set_static_mesh(handle_mesh)
target_marker.set_material(0, handle_material)
target_marker.set_editor_property("relative_location", unreal.Vector(250.0, 0.0, -70.0))
target_marker.set_editor_property("relative_rotation", unreal.Rotator())
target_marker.set_editor_property("relative_scale3d", unreal.Vector(1.5, 1.5, 0.025))
target_marker.set_editor_property("visible", False)
target_marker.set_editor_property("hidden_in_game", True)

unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
unreal.EditorAssetLibrary.save_loaded_asset(hwacha_bp, only_if_is_dirty=False)
unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("SINGIJEON_HANDLE_HIGHLIGHTS CONFIGURE SUCCESS")
