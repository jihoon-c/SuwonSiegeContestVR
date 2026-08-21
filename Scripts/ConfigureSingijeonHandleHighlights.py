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
hwacha_cdo.set_editor_property("aim_completion_distance", 30.0)
hwacha_cdo.set_editor_property("aim_completion_yaw_degrees", 10.0)

# The imported Hwacha mesh runs from about X=-85..183, Y=-69..69 and Z=0..178.
# These slim cylinders overlap the rear wooden handles. Runtime code shows them
# while the loaded Hwacha still needs aiming, hides them during a correct two-hand
# grip, and restores them after an incomplete release.
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

unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
unreal.EditorAssetLibrary.save_loaded_asset(hwacha_bp, only_if_is_dirty=False)
unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("SINGIJEON_HANDLE_HIGHLIGHTS CONFIGURE SUCCESS")
